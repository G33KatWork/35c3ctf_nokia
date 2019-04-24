/* (C) 2017 by Holger Hans Peter Freyther
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <inttypes.h>

#include <osmocom/bb/mobile/gsm322.h>
#include <osmocom/bb/mobile/primitives.h>
#include <osmocom/bb/mobile/app_mobile.h>
#include <osmocom/bb/common/logging.h>
#include <osmocom/bb/common/osmocom_data.h>

#include <osmocom/core/timer.h>
#include <osmocom/core/talloc.h>
#include <osmocom/gsm/protocol/gsm_04_11.h>

static LLIST_HEAD(s_prims);

struct mobile_prim_intf *mobile_prim_intf_alloc(struct osmocom_ms *ms)
{
    struct mobile_prim_intf *intf;

    intf = talloc_zero(ms, struct mobile_prim_intf);
    intf->ms = ms;

    llist_add_tail(&intf->entry, &s_prims);
    return intf;
}

void mobile_prim_intf_free(struct mobile_prim_intf *intf)
{
    llist_del(&intf->entry);
    talloc_free(intf);
}

struct mobile_prim *mobile_prim_alloc(unsigned int primitive, enum osmo_prim_operation op)
{
    struct msgb *msg = msgb_alloc(1024, "Mobile Primitive");
    struct mobile_prim *prim = (struct mobile_prim *) msgb_put(msg, sizeof(*prim));
    osmo_prim_init(&prim->hdr, 0, primitive, op, msg);
    msg->l2h = msg->tail;
    return prim;
}

static void dispatch(struct osmocom_ms *ms, struct mobile_prim *prim)
{
    struct mobile_prim_intf *intf, *tmp;

    llist_for_each_entry_safe(intf, tmp, &s_prims, entry) {
        if (intf->ms == ms)
            intf->indication(intf, prim);
    }
    msgb_free(prim->hdr.msg);
}

void mobile_prim_ntfy_started(struct osmocom_ms *ms, bool started)
{
    struct mobile_prim *prim = mobile_prim_alloc(PRIM_MOB_STARTED, PRIM_OP_INDICATION);

    prim->u.started.started = started;
    dispatch(ms, prim);
}

void mobile_prim_ntfy_shutdown(struct osmocom_ms *ms, int old_state, int new_state)
{
    struct mobile_prim *prim = mobile_prim_alloc(PRIM_MOB_SHUTDOWN, PRIM_OP_INDICATION);

    prim->u.shutdown.old_state = old_state;
    prim->u.shutdown.new_state = new_state;
    dispatch(ms, prim);
}

int mobile_prim_ntfy_sms_new_tpdu(struct osmocom_ms *ms, uint8_t msg_ref, struct msgb *msg)
{
    struct mobile_prim *prim = mobile_prim_alloc(PRIM_MOB_SMS, PRIM_OP_INDICATION);

    if(msgb_l4len(msg) > sizeof(prim->u.sms.tpdu))
        return GSM411_RP_CAUSE_SEMANT_INC_MSG;

    prim->u.sms.msg_ref = msg_ref;
    prim->u.sms.tpdu_len = msgb_l4len(msg);
    memcpy(&prim->u.sms.tpdu, msgb_sms(msg), msgb_l4len(msg));
    dispatch(ms, prim);

    return 0;
}

void mobile_prim_ntfy_sms_status(struct osmocom_ms *ms, uint8_t msg_ref, uint8_t cause)
{
    struct mobile_prim *prim = mobile_prim_alloc(PRIM_MOB_SMS, PRIM_OP_CONFIRM);

    prim->u.sms.msg_ref = msg_ref;
    prim->u.sms.cause_valid = true;
    prim->u.sms.cause = cause;
    dispatch(ms, prim);
}

void mobile_prim_ntfy_mm_status(struct osmocom_ms *ms, int state, int substate, int mr_substate)
{
    struct mobile_prim *prim = mobile_prim_alloc(PRIM_MOB_MM, PRIM_OP_INDICATION);

    prim->u.mm.state = state;
    prim->u.mm.substate = substate;
    prim->u.mm.prev_substate = mr_substate;
    dispatch(ms, prim);
}

void mobile_prim_ntfy_gsm322_status(struct osmocom_ms *ms, int machine, int state)
{
    struct mobile_prim *prim = mobile_prim_alloc(PRIM_MOB_GSM322, PRIM_OP_INDICATION);

    prim->u.gsm322.machine = machine;
    prim->u.gsm322.state = state;
    dispatch(ms, prim);
}

void mobile_prim_ntfy_sim_status(struct osmocom_ms *ms, int cause, int tries_left)
{
    struct mobile_prim *prim = mobile_prim_alloc(PRIM_MOB_SIM, PRIM_OP_INDICATION);

    prim->u.sim.cause = cause;
    prim->u.sim.tries_left = tries_left;
    dispatch(ms, prim);
}

void mobile_prim_ntfy_network_assignment(struct osmocom_ms *ms, uint16_t mcc, uint16_t mnc)
{
    struct mobile_prim *prim = mobile_prim_alloc(PRIM_MOB_NETWORK_ASSIGNMENT, PRIM_OP_INDICATION);

    prim->u.network_assignment.mcc = mcc;
    prim->u.network_assignment.mnc = mnc;
    dispatch(ms, prim);
}

static int send_sms(struct mobile_prim_intf *intf, struct mobile_sms_param *param)
{
    LOGP(DSMS, LOGL_INFO, "Transmitting SMS TPDU (Ref %u): %u %s\n", param->msg_ref, param->tpdu_len, osmo_hexdump(param->tpdu, param->tpdu_len));
    return gsm411_tx_sms_submit(intf->ms, param->msg_ref, param->tpdu_len, param->tpdu);
}

static int handle_sim(struct mobile_prim_intf *intf, struct mobile_sim_param *param)
{
    if(param->cause == PRIM_SIM_ENTER_PIN)
        gsm_subscr_sim_pin(intf->ms, param->pin1, param->pin2, 0);
    else if(param->cause == PRIM_SIM_ENTER_PUC)
        gsm_subscr_sim_pin(intf->ms, param->pin1, param->pin2, 99);

    return 0;
}

int mobile_prim_intf_req(struct mobile_prim_intf *intf, struct mobile_prim *prim)
{
    int rc = 0;

    switch (OSMO_PRIM_HDR(&prim->hdr)) {
    case OSMO_PRIM(PRIM_MOB_STARTED, PRIM_OP_REQUEST):
        rc = mobile_start(intf->ms);
        break;
    case OSMO_PRIM(PRIM_MOB_SHUTDOWN, PRIM_OP_REQUEST):
        rc = mobile_stop(intf->ms, prim->u.shutdown_request.force);
        break;
    case OSMO_PRIM(PRIM_MOB_SMS, PRIM_OP_REQUEST):
        rc = send_sms(intf, &prim->u.sms);
        break;
    case OSMO_PRIM(PRIM_MOB_SIM, PRIM_OP_REQUEST):
        rc = handle_sim(intf, &prim->u.sim);
        break;
    default:
        LOGP(DMOB, LOGL_ERROR, "Unknown primitive: %d\n", OSMO_PRIM_HDR(&prim->hdr));
        break;
    }

    msgb_free(prim->hdr.msg);
    return rc;
}
