#ifndef _PRIMITIVES_H_
#define _PRIMITIVES_H_

#include <stdint.h>

#include <osmocom/core/prim.h>

// FIXME: get rid of struct msgb and this header? Also in baseband
#include <osmocom/core/socket.h>

#include "sms.h"

//FIXME: add __attribute__((packed))? Also baseband

enum {
    MS_SHUTDOWN_NONE = 0,
    MS_SHUTDOWN_IMSI_DETACH = 1,
    MS_SHUTDOWN_WAIT_RESET = 2,
    MS_SHUTDOWN_COMPL = 3,
};

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
    int state;              /*!< The GSM 3.22 event */
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

#endif
