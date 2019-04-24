#ifndef _APDU_H_
#define _APDU_H_

#include <stdint.h>

#include "sim.h"
#include "msg.h"

struct apdu {
    uint8_t class;
    uint8_t inst;
    uint8_t p[3];
    uint8_t data[0];
} __attribute__((packed));

enum apdu_class {
    APDU_CLASS_GSM = 0xa0
};

enum apdu_gsm_instruction {
    APDU_GSM_INST_SELECT            = 0xa4,
    APDU_GSM_INST_STATUS            = 0xf2,
    APDU_GSM_INST_READ_BINARY       = 0xb0,
    APDU_GSM_INST_UPDATE_BINARY     = 0xd6,
    APDU_GSM_INST_READ_RECORD       = 0xb2,
    APDU_GSM_INST_UPDATE_RECORD     = 0xdc,
    APDU_GSM_INST_SEEK              = 0xa2,
    APDU_GSM_INST_INCREASE          = 0x32,
    APDU_GSM_INST_VERIFY_CHV        = 0x20,
    APDU_GSM_INST_CHANGE_CHV        = 0x24,
    APDU_GSM_INST_DISABLE_CHV       = 0x26,
    APDU_GSM_INST_ENABLE_CHV        = 0x28,
    APDU_GSM_INST_UNBLOCK_CHV       = 0x2c,
    APDU_GSM_INST_INVALIDATE        = 0x04,
    APDU_GSM_INST_REHABLILITATE     = 0x44,
    APDU_GSM_INST_RUN_GSM_ALGO      = 0x88,
    APDU_GSM_INST_SLEEP             = 0xfa,
    APDU_GSM_INST_GET_RESPONSE      = 0xc0,
    APDU_GSM_INST_TERMINAL_PROFILE  = 0x10,
    APDU_GSM_INST_ENVELOPE          = 0xc2,
    APDU_GSM_INST_FETCH             = 0x12,
    APDU_GSM_INST_TERMINAL_RESPONSE = 0x14,
};

enum apdu_gsm_status {
    APDU_STAT_NORMAL                = 0x90,
    APDU_STAT_PROACTIVE             = 0x91,
    APDU_STAT_DL_ERROR              = 0x9e,
    APDU_STAT_RESPONSE              = 0x9f,
    APDU_STAT_RESPONSE_TOO          = 0x61,
    APDU_STAT_APP_TK_BUSY           = 0x93,
    APDU_STAT_MEM_PROBLEM           = 0x92,
    APDU_STAT_REFERENCING           = 0x94,
    APDU_STAT_SECURITY              = 0x98,
    APDU_STAT_INCORR_P3             = 0x67,
    APDU_STAT_INCORR_P1_P2          = 0x6b,
    APDU_STAT_UKN_INST              = 0x6d,
    APDU_STAT_WRONG_CLASS           = 0x6e,
    APDU_STAT_TECH_PROBLEM          = 0x6f,
};

enum apdu_ref_error {
    APDU_REF_ERR_NO_EF_SELECTED     = 0x00,
    APDU_REF_ERR_OUT_OF_RANGE       = 0x02,
    APDU_REF_ERR_NOT_FOUND          = 0x04,
    APDU_REF_ERR_INCONSISTENT       = 0x08,
};

enum apdu_security_error {
    APDU_SEC_ERR_NOT_INITIALIZED    = 0x02,
    APDU_SEC_ERR_ACCESS_COND_NOT_FULFILLED = 0x04,
    APDU_SEC_ERR_CHV_STATUS_IN_CONTRADICTION = 0x08,
    APDU_SEC_ERR_CHV_INVALIDATION_IN_CONTRADICTION = 0x10,
    APDU_SEC_ERR_CHV_BLOCKED        = 0x40,
    APDU_SEC_ERR_INCREASE_MAX_REACHED = 0x80
};

// Replies

struct apdu_reply_select_mf_df {
    uint8_t rfu1[2];
    uint16_t free_mem;
    uint16_t file_id;
    uint8_t file_type;
    uint8_t rfu2[5];
    uint8_t len_additional_data;

    uint8_t file_characteristics;
    uint8_t num_df;
    uint8_t num_ef;
    uint8_t num_codes;
    uint8_t rfu3[1];
    uint8_t chv1_status;
    uint8_t unblock_chv1_status;
    uint8_t chv2_status;
    uint8_t unblock_chv2_status;
    uint8_t rfu4[1];
} __attribute__((packed));

struct apdu_reply_select_ef {
    uint8_t rfu1[2];
    uint16_t file_size;
    uint16_t file_id;
    uint8_t file_type;
    uint8_t cyclic_increase_allowed;
    uint8_t access_conditions[3];
    uint8_t file_status;
    uint8_t len_additional_data;

    uint8_t ef_structure;
    uint8_t record_len;
} __attribute__((packed));

struct msg *apdu_process(struct simcard *sim, struct msg *msg);

#endif
