#include <osmocom/bb/common/osmocom_data.h>
#include <osmocom/bb/common/logging.h>

#include <osmocom/core/talloc.h>
#include <osmocom/core/linuxlist.h>
#include <osmocom/core/signal.h>
#include <osmocom/core/application.h>

#include <osmocom/bb/common/l1ctl.h>
#include <osmocom/bb/common/l1l2_interface.h>

#include <osmocom/bb/mobile/gsm480_ss.h>
#include <osmocom/bb/mobile/gsm411_sms.h>
#include <osmocom/bb/mobile/primitives.h>

#include <osmocom/bb/common/sim_socket_interface.h>
#ifdef SIM_TRUSTZONE
#include <osmocom/bb/common/sim_trustzone_interface.h>
#endif

#include <l1ctl_proto.h>

#define _GNU_SOURCE
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <libgen.h>

void *l23_ctx = NULL;
struct llist_head ms_list;
struct gsmtap_inst *gsmtap_inst = NULL;

int mncc_recv_socket(struct osmocom_ms *ms, int msg_type, void *arg);
int mncc_recv_dummy(struct osmocom_ms *ms, int msg_type, void *arg);

void mobile_set_started(struct osmocom_ms *ms, bool state);
void mobile_set_shutdown(struct osmocom_ms *ms, int state);

/* handle ms instance */
int mobile_work(struct osmocom_ms *ms)
{
    int work = 0, w;

    do {
        w = 0;
        w |= gsm48_rsl_dequeue(ms);
        w |= gsm48_rr_dequeue(ms);
        w |= gsm48_mmxx_dequeue(ms);
        w |= gsm48_mmr_dequeue(ms);
        w |= gsm48_mmevent_dequeue(ms);
        w |= gsm322_plmn_dequeue(ms);
        w |= gsm322_cs_dequeue(ms);
        w |= gsm_sim_job_dequeue(ms);
        if (w)
            work = 1;
    } while (w);
    return work;
}

/* run ms instance, if layer1 is available */
int mobile_signal_cb(unsigned int subsys, unsigned int signal,
             void *handler_data, void *signal_data)
{
    struct osmocom_ms *ms;
    struct gsm_settings *set;
    struct msgb *nmsg;

    if (subsys != SS_L1CTL)
        return 0;

    switch (signal) {
    case S_L1CTL_RESET:
        ms = signal_data;
        set = &ms->settings;

        /* waiting for reset after shutdown */
        if (ms->shutdown == MS_SHUTDOWN_WAIT_RESET) {
            LOGP(DMOB, LOGL_NOTICE, "MS '%s' has been resetted\n", ms->name);
            //ms->shutdown = MS_SHUTDOWN_COMPL;
            mobile_set_shutdown(ms, MS_SHUTDOWN_COMPL);
            break;
        }

        if (ms->started)
            break;

        /* insert test card, if enabled */
        switch (set->sim_type) {
        case GSM_SIM_TYPE_TEST:
            LOGP(DMOB, LOGL_NOTICE, "MS '%s' is using test SIM card\n", ms->name);
            gsm_subscr_testcard(ms, set->test_rplmn_mcc,
                set->test_rplmn_mnc, set->test_lac,
                set->test_tmsi, set->test_imsi_attached);
            break;
        case GSM_SIM_TYPE_SOCK:
            LOGP(DMOB, LOGL_NOTICE, "MS '%s' is using socket SIM card\n", ms->name);
            gsm_subscr_socket_sim(ms);
            break;
#ifdef SIM_TRUSTZONE
        case GSM_SIM_TYPE_TRUSTZONE:
            LOGP(DMOB, LOGL_NOTICE, "MS '%s' is using trustzone SIM card\n", ms->name);
            gsm_subscr_trustzone_sim(ms);
            break;
#endif
        default:
            /* no SIM, trigger PLMN selection process */
            nmsg = gsm322_msgb_alloc(GSM322_EVENT_SWITCH_ON);
            if (!nmsg)
                return -ENOMEM;
            gsm322_plmn_sendmsg(ms, nmsg);
            nmsg = gsm322_msgb_alloc(GSM322_EVENT_SWITCH_ON);
            if (!nmsg)
                return -ENOMEM;
            gsm322_cs_sendmsg(ms, nmsg);
        }

        mobile_set_started(ms, true);
    }
    return 0;
}

/* power-off ms instance */
int mobile_exit(struct osmocom_ms *ms, int force)
{
    struct gsm48_mmlayer *mm = &ms->mmlayer;

    /* if shutdown is already performed */
    if (ms->shutdown >= MS_SHUTDOWN_WAIT_RESET)
        return 0;

    if (!force && ms->started) {
        struct msgb *nmsg;

        mobile_set_shutdown(ms, MS_SHUTDOWN_IMSI_DETACH);
        nmsg = gsm48_mmevent_msgb_alloc(GSM48_MM_EVENT_IMSI_DETACH);
        if (!nmsg)
            return -ENOMEM;
        gsm48_mmevent_msg(mm->ms, nmsg);

        return -EBUSY;
    }

    gsm322_exit(ms);
    gsm48_mm_exit(ms);
    gsm48_rr_exit(ms);
    gsm_subscr_exit(ms);
    gsm480_ss_exit(ms);
    gsm411_sms_exit(ms);
    gsm_sim_exit(ms);
    lapdm_channel_exit(&ms->lapdm_channel);

    if (ms->started) {
        mobile_set_shutdown(ms, MS_SHUTDOWN_WAIT_RESET); /* being down, wait for reset */
        l1ctl_tx_reset_req(ms, L1CTL_RES_T_FULL);
    } else {
        mobile_set_shutdown(ms, MS_SHUTDOWN_COMPL); /* being down */
    }
    LOGP(DMOB, LOGL_NOTICE, "Power off! (MS %s)\n", ms->name);

    return 0;
}

/* power-on ms instance */
int mobile_start(struct osmocom_ms *ms)
{
    int rc;

    gsm_settings_arfcn(ms);

    lapdm_channel_init(&ms->lapdm_channel, LAPDM_MODE_MS);
    ms->lapdm_channel.lapdm_dcch.datalink[DL_SAPI3].dl.t200_sec = T200_DCCH_SHARED;
    ms->lapdm_channel.lapdm_dcch.datalink[DL_SAPI3].dl.t200_usec = 0;
    ms->lapdm_channel.lapdm_acch.datalink[DL_SAPI3].dl.t200_sec = T200_ACCH;
    ms->lapdm_channel.lapdm_acch.datalink[DL_SAPI3].dl.t200_usec = 0;
    lapdm_channel_set_l1(&ms->lapdm_channel, l1ctl_ph_prim_cb, ms);

    gsm_sim_init(ms);
    gsm480_ss_init(ms);
    gsm411_sms_init(ms);
    gsm_subscr_init(ms);
    gsm48_rr_init(ms);
    gsm48_mm_init(ms);
    INIT_LLIST_HEAD(&ms->trans_list);
    gsm322_init(ms);

    gsm_random_imei(&ms->settings);

    mobile_set_shutdown(ms, MS_SHUTDOWN_NONE);
    mobile_set_started(ms, false);

    if (!strcmp(ms->settings.imei, "000000000000000")) {
        LOGP(DMOB, LOGL_NOTICE, "***\nWarning: Mobile '%s' has default IMEI: %s\n",
            ms->name, ms->settings.imei);
        LOGP(DMOB, LOGL_NOTICE, "This could relate your identitiy to other users with "
            "default IMEI.\n***\n");
    }

    rc = layer2_open(ms, ms->settings.layer2_socket_path);
    if (rc < 0) {
        LOGP(DMOB, LOGL_ERROR, "Failed during layer2_open()\n");
        ms->l2_wq.bfd.fd = -1;
        mobile_exit(ms, 1);
        return rc;
    }

    l1ctl_tx_reset_req(ms, L1CTL_RES_T_FULL);
    LOGP(DMOB, LOGL_NOTICE, "Mobile '%s' initialized, please start phone now!\n", ms->name);
    return 0;
}

int mobile_stop(struct osmocom_ms *ms, int force)
{
    if (force && ms->shutdown <= MS_SHUTDOWN_IMSI_DETACH)
        return mobile_exit(ms, 1);
    if (!force && ms->shutdown == MS_SHUTDOWN_NONE)
        return mobile_exit(ms, 0);
    return 0;
}

/* create ms instance */
struct osmocom_ms *mobile_new(char *name, int (*ui_recv_socket)(struct osmocom_ms *ms, struct msgb *msg))
{
    static struct osmocom_ms *ms;
    char *ui_name;

    ms = talloc_zero(l23_ctx, struct osmocom_ms);
    if (!ms) {
        LOGP(DMOB, LOGL_ERROR, "Failed to allocate MS: %s\n", name);
        return NULL;
    }

    talloc_set_name(ms, "ms_%s", name);
    ms->name = talloc_strdup(ms, name);
    ms->l2_wq.bfd.fd = -1;

    /* Register a new MS */
    llist_add_tail(&ms->entity, &ms_list);

    gsm_support_init(ms);
    gsm_settings_init(ms);

    //mobile_set_shutdown(ms, MS_SHUTDOWN_COMPL);
    ms->shutdown = MS_SHUTDOWN_COMPL;

    ui_name = talloc_asprintf(ms, "/tmp/ms_ui_%s", ms->name);
    ms->ui_entity.ui_recv = ui_recv_socket;
    ms->ui_entity.sock_state = ui_sock_init(ms, ui_name);
    talloc_free(ui_name);

    return ms;
}

/* destroy ms instance */
int mobile_delete(struct osmocom_ms *ms, int force)
{
    int rc;

    ms->deleting = true;

    //mncc_sock_exit(ms->mncc_entity.sock_state);
    //ms->mncc_entity.sock_state = NULL;

    if (ms->shutdown == MS_SHUTDOWN_NONE || (ms->shutdown == MS_SHUTDOWN_IMSI_DETACH && force)) {
        rc = mobile_exit(ms, force);
        if (rc < 0)
            return rc;
    }

    return 0;
}

void mobile_set_started(struct osmocom_ms *ms, bool state)
{
    ms->started = state;

    mobile_prim_ntfy_started(ms, state);
}

void mobile_set_shutdown(struct osmocom_ms *ms, int state)
{
    int old_state = ms->shutdown;
    ms->shutdown = state;

    mobile_prim_ntfy_shutdown(ms, old_state, state);
}

void app_init()
{
    srand(time(NULL));
    INIT_LLIST_HEAD(&ms_list);

    l23_ctx = talloc_named_const(NULL, 1, "layer2 context");
    /* TODO: measure and choose a proper pool size */
    msgb_talloc_ctx_init(l23_ctx, 0);

    /* Init default stderr logging */
    osmo_init_logging2(l23_ctx, &log_info);

    //log_set_all_filter(osmo_stderr_target, 1);
    //log_set_log_level(osmo_stderr_target, LOGL_DEBUG);
    log_set_category_filter(osmo_stderr_target, DLSMS, 1, LOGL_DEBUG);
    log_set_category_filter(osmo_stderr_target, DSIM, 1, LOGL_DEBUG);

    osmo_signal_register_handler(SS_L1CTL, &mobile_signal_cb, NULL);
    osmo_signal_register_handler(SS_L1CTL, &gsm322_l1_signal, NULL);
}

void app_shutdown()
{
    osmo_signal_unregister_handler(SS_L1CTL, &gsm322_l1_signal, NULL);
    osmo_signal_unregister_handler(SS_L1CTL, &mobile_signal_cb, NULL);

    log_fini();
}

int app_dowork()
{
    struct osmocom_ms *ms, *ms2;
    int work = 0;
    
    llist_for_each_entry_safe(ms, ms2, &ms_list, entity) {
        if (ms->shutdown != MS_SHUTDOWN_COMPL)
            work |= mobile_work(ms);
        if (ms->shutdown == MS_SHUTDOWN_COMPL) {
            if (ms->l2_wq.bfd.fd > -1) {
                layer2_close(ms);
                ms->l2_wq.bfd.fd = -1;
            }

            socketcard_close(ms);
#ifdef SIM_TRUSTZONE
            trustzonecard_close(ms);
#endif

            if (ms->deleting) {
                gsm_settings_exit(ms);
                llist_del(&ms->entity);
                talloc_free(ms);
                work = 1;
            }
        }
    }

    osmo_select_main(1);

    return work;
}
