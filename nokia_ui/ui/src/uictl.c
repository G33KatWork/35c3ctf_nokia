#include "uictl.h"

#include "networks.h"
#include "gsm48_mm.h"
#include "gsm322.h"
#include "ui_sock.h"
#include "ui_event.h"
#include "sms_receive.h"

#include <string.h>

struct msgb* uictl_msgb_alloc(void)
{
    struct msgb *msg;

    msg = msgb_alloc(sizeof(struct mobile_prim), "mobile_prim_ui_to_bb");
    //OSMO_ASSERT(msg);

    return msg;
}

void uictl_send_mobile_start(struct nokia_ui *ui)
{
    struct msgb *msg = uictl_msgb_alloc();
    struct mobile_prim *prim_msg = (struct mobile_prim *)msgb_put(msg, sizeof(*prim_msg));

    prim_msg->hdr.primitive = PRIM_MOB_STARTED;
    prim_msg->hdr.operation = PRIM_OP_REQUEST;

    bb_sock_from_ui(ui->bb_entity.sock_state, msg);
}

void uictl_send_mobile_shutdown(struct nokia_ui *ui, uint8_t force)
{
    struct msgb *msg = uictl_msgb_alloc();
    struct mobile_prim *prim_msg = (struct mobile_prim *)msgb_put(msg, sizeof(*prim_msg));

    prim_msg->hdr.primitive = PRIM_MOB_SHUTDOWN;
    prim_msg->hdr.operation = PRIM_OP_REQUEST;
    prim_msg->u.shutdown_request.force = force;

    bb_sock_from_ui(ui->bb_entity.sock_state, msg);
}

void uictl_send_tpdu(struct nokia_ui *ui, uint8_t msg_ref, uint8_t tpdu_len, uint8_t *tpdu)
{
    struct msgb *msg = uictl_msgb_alloc();
    struct mobile_prim *prim_msg = (struct mobile_prim *)msgb_put(msg, sizeof(*prim_msg));

    prim_msg->hdr.primitive = PRIM_MOB_SMS;
    prim_msg->hdr.operation = PRIM_OP_REQUEST;

    prim_msg->u.sms.tpdu_len = tpdu_len;
    prim_msg->u.sms.msg_ref = msg_ref;
    memcpy(prim_msg->u.sms.tpdu, tpdu, tpdu_len);

    fprintf(stderr, "Submitting RAW TPDU: (%u) %s\n", prim_msg->u.sms.tpdu_len, osmo_hexdump(prim_msg->u.sms.tpdu, prim_msg->u.sms.tpdu_len));

    bb_sock_from_ui(ui->bb_entity.sock_state, msg);
}

void uictl_send_pin(struct nokia_ui *ui, char *pin1, char *pin2, int cause)
{
    struct msgb *msg = uictl_msgb_alloc();
    struct mobile_prim *prim_msg = (struct mobile_prim *)msgb_put(msg, sizeof(*prim_msg));

    prim_msg->hdr.primitive = PRIM_MOB_SIM;
    prim_msg->hdr.operation = PRIM_OP_REQUEST;

    prim_msg->u.sim.cause = cause;
    if(pin1) memcpy(prim_msg->u.sim.pin1, pin1, 8);
    if(pin2) memcpy(prim_msg->u.sim.pin2, pin2, 8);

    fprintf(stderr, "Submitting SIM PIN message - cause: %u\n", prim_msg->u.sim.cause);

    bb_sock_from_ui(ui->bb_entity.sock_state, msg);
}

int uictl_bb_sock_to_ui_cb(struct nokia_ui *ui, struct msgb *msg)
{
    //printf("(%s)\n", osmo_hexdump(msg->tail, msgb_tailroom(msg)));

    struct mobile_prim *prim = (struct mobile_prim *)msg->tail;
    struct osmo_prim_hdr *hdr = &prim->hdr;

    struct mobile_mm_param *mm_param = &prim->u.mm;
    struct mobile_gsm322_param *gsm322_param = &prim->u.gsm322;
    struct mobile_sim_param *sim = &prim->u.sim;
    struct mobile_network_assignment *network_assignment = &prim->u.network_assignment;
    struct mobile_sms_param *sms = &prim->u.sms;

    printf("Received BB traffic. Primitive: %s - Operation: %s\n", prim_to_str(hdr->primitive), prim_op_to_str(hdr->operation));

    switch(OSMO_PRIM_HDR(&prim->hdr)) {
        case OSMO_PRIM(PRIM_MOB_SHUTDOWN, PRIM_OP_INDICATION):
            printf("Shutdown indication: Old state: %u - New State: %u\n", prim->u.shutdown.old_state, prim->u.shutdown.new_state);
            if(prim->u.shutdown.new_state == MS_SHUTDOWN_COMPL)
                ui->ui_entity.quit = 1;
            break;

        case OSMO_PRIM(PRIM_MOB_MM, PRIM_OP_INDICATION):
            printf("MM indication: State: %u - Substate: %u - Prev Substate: %u\n", mm_param->state, mm_param->substate, mm_param->prev_substate);
            break;

        case OSMO_PRIM(PRIM_MOB_GSM322, PRIM_OP_INDICATION):
            printf("GSM322 indication: Machine: %u State: %u\n", gsm322_param->machine, gsm322_param->state);

            switch(gsm322_param->machine)
            {
                case GSM322_STATE_MACHINE_PLMN_AUTOMATIC:
                    switch(gsm322_param->state)
                    {
                        case GSM322_A2_ON_PLMN:
                            //ui->ui_entity.on_network = true;
                            break;

                        default:
                            ui->ui_entity.on_network = false;
                            break;
                    }
                    break;

                case GSM322_STATE_MACHINE_PLMN_MANUAL:
                    switch(gsm322_param->state)
                    {
                        case GSM322_M2_ON_PLMN:
                            //ui->ui_entity.on_network = true;
                            break;

                        default:
                            ui->ui_entity.on_network = false;
                            break;
                    }
                    break;
            }
            break;

        case OSMO_PRIM(PRIM_MOB_SIM, PRIM_OP_INDICATION):
            printf("SIM indication. Cause: %u\n", sim->cause);
            struct msgb *msg = ui_event_msgb_alloc(UI_EVENT_SIM);
            struct ui_event_msg *ui_event = (struct ui_event_msg *)msg->data;
            ui_event->u.sim_params.cause = sim->cause;
            ui_event->u.sim_params.tries_left = sim->tries_left;
            ui_event_sendmsg(ui, msg);
            break;

        case OSMO_PRIM(PRIM_MOB_NETWORK_ASSIGNMENT, PRIM_OP_INDICATION):
            printf("Assigned to network: 0x%x 0x%x\n", network_assignment->mcc, network_assignment->mnc);
            ui->ui_entity.on_network = true;
            ui->ui_entity.mcc = network_assignment->mcc;
            ui->ui_entity.mnc = network_assignment->mnc;
            break;

        case OSMO_PRIM(PRIM_MOB_SMS, PRIM_OP_INDICATION):
            printf("Received SMS TPDU: %s\n", osmo_hexdump(sms->tpdu, sizeof(sms->tpdu)));
            sms_receive(ui, sms->tpdu_len, sms->tpdu);
            break;

        case OSMO_PRIM(PRIM_MOB_SMS, PRIM_OP_CONFIRM):
            printf("Received SMS confirmation: msg_ref: %u - cause: %u\n", sms->msg_ref, sms->cause);
            if(sms->cause_valid)
                sms_notify_job_completion(ui, sms->msg_ref, sms->cause);
            break;
    }

    return 0;
}

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
        case PRIM_MOB_SIM:
            return "SIM";
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
