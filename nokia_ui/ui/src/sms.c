#include "sms.h"

#include <string.h>

#include <osmocom/core/utils.h>
#include <osmocom/core/talloc.h>
#include <osmocom/gsm/gsm_utils.h>
#include <osmocom/gsm/gsm0411_utils.h>
#include <osmocom/gsm/protocol/gsm_04_11.h>

#include "uictl.h"
#include "ui_event.h"

#define SMS_MAX_RETRY           10
#define SMS_SEND_RETRY_TIMEOUT  10

struct sms_job {
    uint8_t tpdu[163];
    uint8_t tpdu_len;
    uint8_t msg_ref;
    struct nokia_ui *ui;
    struct osmo_timer_list send_timer;
    int send_retry_counter;
    struct llist_head entry;
};

static int gsm340_gen_tpdu(uint8_t *smsp, uint8_t msg_ref, const char *recipient, uint8_t data_coding_scheme, uint8_t user_data_len, uint8_t *user_data, bool udh_present);
static void sms_enqueue_send_simple(struct nokia_ui *ui, const char *recipient, const char *text);
static void timeout_send_retry(void *arg);
static void sms_job_submit_to_baseband(struct nokia_ui *ui, struct sms_job *sms);
static void sms_job_dispatch_ui_event(struct nokia_ui *ui, uint8_t msg_ref, int cause);

static uint8_t next_msg_ref = 0;
static uint8_t next_csms_ref = 0;
static LLIST_HEAD(sms_jobs);

int gsm_7bit_encode_n_padding(uint8_t *result, size_t n, const char *data, int *octets, uint8_t padding)
{
    int y = 0;
    int o;
    size_t max_septets = n * 8 / 7;

    /* prepare for the worst case, every character expanding to two bytes */
    uint8_t *rdata = calloc(strlen(data) * 2, sizeof(uint8_t));
    y = gsm_septet_encode(rdata, data);

    if (y > max_septets) {
        /*
         * Limit the number of septets to avoid the generation
         * of more than n octets.
         */
        y = max_septets;
    }

    o = gsm_septets2octets(result, rdata, y, padding);

    if (octets)
        *octets = o;

    free(rdata);

    /*
     * We don't care about the number of octets, because they are not
     * unique. E.g.:
     *  1.) 46 non-extension characters + 1 extension character
     *         => (46 * 7 bit + (1 * (2 * 7 bit))) / 8 bit =  42 octets
     *  2.) 47 non-extension characters
     *         => (47 * 7 bit) / 8 bit = 41,125 = 42 octets
     *  3.) 48 non-extension characters
     *         => (48 * 7 bit) / 8 bit = 42 octects
     */
    return y;
}

static int gsm340_gen_tpdu(uint8_t *smsp, uint8_t msg_ref, const char *recipient, uint8_t data_coding_scheme, uint8_t user_data_len, uint8_t *user_data, bool udh_present)
{
    uint8_t *smsp_orig = smsp;
    uint8_t da[12]; /* max len per 03.40 */
    uint8_t da_len = 0;
    uint8_t octet_len;
    uint8_t sms_vpf = GSM340_TP_VPF_NONE;
    uint8_t sms_vp;

    /* TP-MTI (message type indicator) */
    *smsp = GSM340_SMS_SUBMIT_MS2SC;

    /* TP-RD */
    if (0 /* FIXME */)
        *smsp |= 0x04;

    /* TP-VPF */
    *smsp |= (sms_vpf << 3);

    /* TP-SRI(deliver)/SRR(submit) */
    if (0 /*sms->status_rep_req*/)
        *smsp |= 0x20;

    /* TP-UDHI (indicating TP-UD contains a header) */
    if (udh_present)
        *smsp |= 0x40;

    /* TP-RP */
    if (0 /*sms->reply_path_req*/)
        *smsp |= 0x80;

    smsp++;


    /* generate message ref */
    *smsp = msg_ref;
    smsp++;


    /* generate destination address */
    if (recipient[0] == '+')
        da_len = gsm340_gen_oa(da, sizeof(da), 0x1, 0x1, recipient + 1);
    else
        da_len = gsm340_gen_oa(da, sizeof(da), 0x0, 0x1, recipient);

    memcpy(smsp, da, da_len);
    smsp += da_len;


    /* generate TP-PID */
    (*smsp) = 0;
    smsp++;


    /* generate TP-DCS */
    *smsp = data_coding_scheme;
    smsp++;


    /* generate TP-VP */
    // switch (sms_vpf) {
    // case GSM340_TP_VPF_NONE:
    //     sms_vp = 0;
    //     break;
    // default:
    //     fprintf(stderr, "VPF unsupported, please fix!\n");
    //     exit(0);
    // }
    // smsp = msgb_put(msg, sms_vp);


    /* generate TP-UDL */
    *smsp = user_data_len;
    smsp++;

    /* Copy payload */
    octet_len = user_data_len*7/8;
    if (user_data_len*7%8 != 0)
        octet_len++;
    memcpy(smsp, user_data, octet_len);
    smsp += octet_len;

    return smsp - smsp_orig;
}

static void sms_enqueue_send_simple(struct nokia_ui *ui, const char *recipient, const char *text)
{
    struct sms_job *sms = talloc_zero(ui, struct sms_job);
    if(!sms)
        return;

    uint8_t user_data[140];
    uint8_t user_data_len = gsm_7bit_encode_n(user_data, sizeof(user_data), text, NULL);

    sms->tpdu_len = gsm340_gen_tpdu(sms->tpdu, next_msg_ref, recipient, DCS_7BIT_DEFAULT, user_data_len, user_data, false);
    sms->msg_ref = next_msg_ref;
    sms->send_retry_counter = 0;
    sms->ui = ui;

    next_msg_ref += 1;
    llist_add(&sms->entry, &sms_jobs);
}

static void sms_enqueue_send_multipart(struct nokia_ui *ui, const char *recipient, const char *text)
{
    uint8_t csms_ref_num = next_csms_ref;
    csms_ref_num++;

    uint8_t user_data[140] = {0};
    char cur_text[153+1];

    int num_parts = ((strlen(text) - 1)/153) + 1;
    for(int i = 0; i < num_parts; i++) {
        struct sms_job *sms = talloc_zero(ui, struct sms_job);
        if(!sms)
            return;

        uint8_t user_data_len_oct = 6;

        //build the UDH
        uint8_t *udh = &user_data[0];
        uint8_t *payload = &user_data[user_data_len_oct];
        
        udh[0] = 0x05; //Length
        udh[1] = 0x00; //Information element identifier: 0 = concatenated SMS
        udh[2] = 0x03; //More length
        udh[3] = csms_ref_num; //CSMS reference number
        udh[4] = (uint8_t)num_parts; //Total number of parts
        udh[5] = (uint8_t)i+1; //Part number

        memcpy(cur_text, &text[153*i], 153);
        cur_text[153] = 0;

        int udh_padding = user_data_len_oct*8 % 7;
        if(udh_padding) udh_padding = 7 - udh_padding;

        int text_septet_len = gsm_7bit_encode_n_padding(payload, sizeof(user_data)-user_data_len_oct, cur_text, NULL, udh_padding);
        sms->tpdu_len = gsm340_gen_tpdu(sms->tpdu, next_msg_ref, recipient, DCS_7BIT_DEFAULT, (user_data_len_oct*8+udh_padding)/7 + text_septet_len, user_data, true);
        sms->msg_ref = next_msg_ref;
        sms->send_retry_counter = 0;
        sms->ui = ui;

        next_msg_ref += 1;
        llist_add_tail(&sms->entry, &sms_jobs);
    }
 }

static void timeout_send_retry(void *arg)
{
    struct sms_job *sms = arg;

    printf("Timer send retry for SMS 0x%02x has fired\n", sms->msg_ref);
    if(sms->send_retry_counter == SMS_MAX_RETRY) {
        printf("SMS sending for SMS 0x%02x failed after 10 consecutive tries\n", sms->send_retry_counter);
        sms_job_dispatch_ui_event(sms->ui, sms->msg_ref, 1234); //FIXME: remove dummy cause
        osmo_timer_del(&sms->send_timer);
        llist_del(&sms->entry);
        talloc_free(sms);
    } else {
        sms->send_retry_counter += 1;
        printf("Retrying SMS sending for SMS 0x%02x for the %uth time\n", sms->msg_ref, sms->send_retry_counter);
        sms_job_submit_to_baseband(sms->ui, sms);
    }
}

static void sms_job_submit_to_baseband(struct nokia_ui *ui, struct sms_job *sms)
{
    printf("Submitting SMS 0x%02x to baseband\n", sms->msg_ref);
    uictl_send_tpdu(ui, sms->msg_ref, sms->tpdu_len, sms->tpdu);
    sms->send_timer.data = sms;
    sms->send_timer.cb = timeout_send_retry;
    osmo_timer_schedule(&sms->send_timer, SMS_SEND_RETRY_TIMEOUT, 0);
}

static void sms_job_dispatch_ui_event(struct nokia_ui *ui, uint8_t msg_ref, int cause)
{
    struct msgb *msg = ui_event_msgb_alloc(UI_EVENT_SMS_SEND_DONE);
    struct ui_event_msg *ui_event = (struct ui_event_msg *)msg->data;

    ui_event->u.sms_done_params.msg_ref = msg_ref;
    ui_event->u.sms_done_params.cause = cause;

    ui_event_sendmsg(ui, msg);
}

void sms_process_jobs(struct nokia_ui *ui)
{
    struct sms_job *sms, *sms2;

    llist_for_each_entry_safe(sms, sms2, &sms_jobs, entry) {
        if(!osmo_timer_pending(&sms->send_timer)) {
            printf("Initially submitting SMS job 0x%02x to baseband\n", sms->msg_ref);
            sms_job_submit_to_baseband(ui, sms);
        }
    }
}

void sms_notify_job_completion(struct nokia_ui *ui, uint8_t msg_ref, int cause)
{
    struct sms_job *sms, *sms2;

    llist_for_each_entry_safe(sms, sms2, &sms_jobs, entry) {
        if(sms->msg_ref == msg_ref && cause == 0) {
            printf("SMS send for SMS 0x%02x succeeded\n", sms->msg_ref);
            sms_job_dispatch_ui_event(ui, sms->msg_ref, cause);
            osmo_timer_del(&sms->send_timer);
            llist_del(&sms->entry);
            talloc_free(sms);
        }
    }
}

void sms_send(struct nokia_ui *ui, const char* recipient, const char* text)
{
    size_t sms_len = strlen(text);

    if(sms_len <= 160)
        sms_enqueue_send_simple(ui, recipient, text);
    else
        sms_enqueue_send_multipart(ui, recipient, text);
}
