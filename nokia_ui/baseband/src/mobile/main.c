#include <osmocom/bb/mobile/app_mobile.h>

#include <osmocom/bb/common/osmocom_data.h>
#include <osmocom/bb/common/logging.h>
#include <osmocom/bb/mobile/primitives.h>

#include <osmocom/crypt/auth.h>

#include <string.h>

struct mobile_prim_intf* mobile_prim_intf = NULL;

const char* prim_to_str(enum mobile_prims p)
{
    switch(p) {
        case PRIM_MOB_STARTED:
            return "MOB_STARTED";
        case PRIM_MOB_SHUTDOWN:
            return "MOB_SHUTDOWN";
        case PRIM_MOB_SMS:
            return "SMS";
        case PRIM_MOB_MM:
            return "MM";
        case PRIM_MOB_GSM322:
            return "GSM322";
        case PRIM_MOB_NETWORK_ASSIGNMENT:
            return "NETWORK_ASSIGNMENT";
        default:
            return "!!!UNKNOWN!!!";
    }
}

const char* prim_op_to_str(enum osmo_prim_operation o)
{
    switch(o) {
        case PRIM_OP_REQUEST:
            return "REQUEST";
        case PRIM_OP_RESPONSE:
            return "RESPONSE";
        case PRIM_OP_INDICATION:
            return "INDICATION";
        case PRIM_OP_CONFIRM:
            return "CONFIRM";
        default:
            return "!!!UNKNOWN!!!";
    }
}


int ui_sock_to_bb(struct osmocom_ms *ms, struct msgb *msg)
{
    DEBUGP(DUI, "UI -> BB (%s)\n", osmo_hexdump(msg->tail, msgb_tailroom(msg)));

    // Forward message to baseband primitive code
    return mobile_prim_intf_req(mobile_prim_intf, (struct mobile_prim *)msg->tail);
}

void mobile_prim_indication(struct mobile_prim_intf *intf, struct mobile_prim *prim)
{
    struct osmo_prim_hdr *hdr = &prim->hdr;

    DEBUGP(DUI, "Received Indication for sending to UI. Primitive: %s - Operation: %s\n", prim_to_str(hdr->primitive), prim_op_to_str(hdr->operation));

    // Forward message to UI socket
    struct msgb *msg;
    struct mobile_prim *prim_send;

    msg = msgb_alloc(sizeof(struct mobile_prim), "mobile_prim_bb_to_ui");
    prim_send = (struct mobile_prim *) msgb_put(msg, sizeof(*prim_send));
    memcpy(prim_send, prim, sizeof(*prim_send));

    int rc = ui_sock_from_baseband(intf->ms->ui_entity.sock_state, msg);
    if(rc < 0) {
        LOGP(DUI, LOGL_ERROR, "Sending primtive message to UI failed: %d\n", rc);
    }

}


int baseband_init()
{
    int rc = 0;
    struct osmocom_ms *ms = mobile_new("nokia", ui_sock_to_bb);
    if(!ms)
        return -1;

    //FIXME: load config here into MS
    struct gsm_settings *set = &ms->settings;

    strcpy(set->imei,      "123456789012347");
    strcpy(set->imeisv,    "1234567890123456");
    strcpy(set->sms_sca,   "12345");

    strcpy(set->test_imsi, "001010000000001");
    memcpy(set->test_ki, "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f", sizeof(set->test_ki));
    set->test_rplmn_mcc = 001;
    set->test_rplmn_mnc = 01;
    set->test_lac = 24;

    set->test_ki_type = OSMO_AUTH_ALG_COMP128v1;

    set->test_rplmn_valid = 1;
    //set->sim_type = GSM_SIM_TYPE_TEST;

    set->sim_type = GSM_SIM_TYPE_SOCK;
#ifdef SIM_TRUSTZONE
    set->sim_type = GSM_SIM_TYPE_TRUSTZONE;
#endif

    mobile_prim_intf = mobile_prim_intf_alloc(ms);
    if(!mobile_prim_intf)
        return -1;

    mobile_prim_intf->indication = mobile_prim_indication;

    return rc;
}

int main(int argc, char **argv)
{
    app_init();
    baseband_init();

    while (1) {
        app_dowork();
        osmo_select_main(0);
    }

    app_shutdown();
}
