#pragma once

#include <osmocom/bb/mobile/gsm411_sms.h>
#include <osmocom/bb/mobile/subscriber.h>

#include <osmocom/core/prim.h>

struct mobile_prim;

enum PRIM_SIM_CAUSE {
    PRIM_SIM_VALID_UNLOCKED,
    PRIM_SIM_INVALID_MISSING,
    PRIM_SIM_PIN1_REQUIRED,
    PRIM_SIM_PIN1_BLOCKED,
    PRIM_SIM_PUC_BLOCKED,
    PRIM_SIM_ENTER_PIN,
    PRIM_SIM_ENTER_PUC,
};

/**
 * Mobile Script<->App primitives. Application script will receive
 * indications and will send primitives to the lower layers. Here
 * we will convert from internal state/events to the primitives. In
 * the future the indications might be generated at lower levels
 * directly.
 */
enum mobile_prims {
    PRIM_MOB_STARTED,
    PRIM_MOB_SHUTDOWN,
    PRIM_MOB_SMS,
    PRIM_MOB_MM,
    PRIM_MOB_GSM322,
    PRIM_MOB_SIM,
    PRIM_MOB_NETWORK_ASSIGNMENT,
};

struct mobile_prim_intf {
    struct osmocom_ms *ms;
    void (*indication)(struct mobile_prim_intf *, struct mobile_prim *prim);

    /* Internal state */
    struct llist_head entry;
};

/**
 * Primitive to indicate starting of the mobile.
 */
struct mobile_started_param {
    bool started;
};

/**
 * Primitive to indicate shutdown of the mobile. It will go through
 * various states.
 */
struct mobile_shutdown_param {
    int old_state;
    int new_state;
};

/**
 * Primitive to request shutdown of the mobile.
 */
struct mobile_shutdown_request_param {
    int force;
};

/**
 * SMS related configs.
 */
struct mobile_sms_param {
    uint8_t tpdu_len;
    uint8_t tpdu[163];

    uint8_t msg_ref;
    bool cause_valid;
    int cause;
};

/**
 * Mobility Management (MM) state changes.
 */
struct mobile_mm_param {
    int state;          /*!< The new MM state */
    int substate;           /*!< The current substate */
    int prev_substate;      /*!< The previous substate */
};

/**
 * GSM322 event notification.
 */
struct mobile_gsm322_param {
    int machine;            /*!< The GSM 3.22 state machine */
    int state;              /*!< The GSM 3.22 state */
};

/**
 * SIM event notification.
 */
struct mobile_sim_param {
    int cause;
    int tries_left;
    char pin1[9];
    char pin2[9];
};

/**
 * Assigned network
 */
struct mobile_network_assignment {
    uint16_t mcc;
    uint16_t mnc;
};

struct mobile_prim {
    struct osmo_prim_hdr hdr;   /*!< Primitive base class */
    union {
        struct mobile_started_param started;
        struct mobile_shutdown_param shutdown;
        struct mobile_shutdown_request_param shutdown_request;
        struct mobile_sms_param sms;
        struct mobile_mm_param mm;
        struct mobile_gsm322_param gsm322;
        struct mobile_sim_param sim;
        struct mobile_network_assignment network_assignment;
    } u;
};


struct mobile_prim_intf *mobile_prim_intf_alloc(struct osmocom_ms *ms);
int mobile_prim_intf_req(struct mobile_prim_intf *intf, struct mobile_prim *hdr);
void mobile_prim_intf_free(struct mobile_prim_intf *intf);

struct mobile_prim *mobile_prim_alloc(unsigned int primitive, enum osmo_prim_operation op);

void mobile_prim_ntfy_started(struct osmocom_ms *ms, bool started);
void mobile_prim_ntfy_shutdown(struct osmocom_ms *ms, int old_state, int new_state);
int mobile_prim_ntfy_sms_new_tpdu(struct osmocom_ms *ms, uint8_t msg_ref, struct msgb *msg);
void mobile_prim_ntfy_sms_status(struct osmocom_ms *ms, uint8_t msg_ref, uint8_t cause);
void mobile_prim_ntfy_mm_status(struct osmocom_ms *ms, int state, int subs, int old_subs);
void mobile_prim_ntfy_network_assignment(struct osmocom_ms *ms, uint16_t mcc, uint16_t mnc);
void mobile_prim_ntfy_gsm322_status(struct osmocom_ms *ms, int machine, int state);
void mobile_prim_ntfy_sim_status(struct osmocom_ms *ms, int cause, int tries_left);
