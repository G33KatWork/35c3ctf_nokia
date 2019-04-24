#pragma once

#include <stdint.h>
#include <osmocom/core/msgb.h>
#include <l1ctl_proto.h>
#include <virtphy/virtual_um.h>
#include <virtphy/l1ctl_sock.h>
#include <virtphy/virt_l1_model.h>

/* following sizes are used for message allocation */
/* size of layer 3 header */
#define L3_MSG_HEAD 4
/* size of layer 3 payload */
#define L3_MSG_DATA 200
#define L3_MSG_SIZE (sizeof(struct l1ctl_hdr) + L3_MSG_HEAD + L3_MSG_DATA)

/* lchan link ID */
#define LID_SACCH 		0x40
#define LID_DEDIC 		0x00

/* signature strengths for the ms */
#define MIN_SIG_LEV_DBM -110
#define MAX_SIG_LEV_DBM -63

/* Ignore all flags of the arfcn */
#define ARFCN_NO_FLAGS_MASK 0x0fff


void l1ctl_sap_init(struct l1_model_ms *model);
void l1ctl_sap_exit(struct l1_model_ms *model);
void prim_pm_init(struct l1_model_ms *model);
void prim_pm_exit(struct l1_model_ms *model);
void l1ctl_sap_tx_to_l23_inst(struct l1_model_ms *model, struct msgb *msg);
void l1ctl_sap_rx_from_l23_inst_cb(struct l1ctl_sock_client *lsc, struct msgb *msg);
void l1ctl_sap_rx_from_l23(struct msgb *msg);
void l1ctl_sap_handler(struct l1_model_ms *ms, struct msgb *msg);

/* utility methods */
struct msgb *l1ctl_msgb_alloc(uint8_t msg_type);
struct msgb *l1ctl_create_l2_msg(int msg_type, uint32_t fn, uint16_t snr,
                                 uint16_t arfcn);

/* receive routines */
void l1ctl_rx_fbsb_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_dm_est_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_dm_rel_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_param_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_dm_freq_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_crypto_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_rach_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_data_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_pm_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_reset_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_ccch_mode_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_tch_mode_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_neigh_pm_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_traffic_req(struct l1_model_ms *, struct msgb *msg);
void l1ctl_rx_sim_req(struct l1_model_ms *, struct msgb *msg);

/* transmit routines */
void l1ctl_tx_reset(struct l1_model_ms *, uint8_t msg_type, uint8_t reset_type);
void l1ctl_tx_rach_conf(struct l1_model_ms *, uint32_t fn, uint16_t arfcn);
void l1ctl_tx_data_conf(struct l1_model_ms *, uint32_t fn, uint16_t snr, uint16_t arfcn);
void l1ctl_tx_data_ind(struct l1_model_ms *, struct msgb *msg, uint16_t arfcn, uint8_t link_id,
                       uint8_t chan_nr, uint32_t fn, uint8_t snr,
                       uint8_t signal_dbm, uint8_t num_biterr,
                       uint8_t fire_crc);
void l1ctl_tx_traffic_conf(struct l1_model_ms *, uint32_t fn, uint16_t snr, uint16_t arfcn);
void l1ctl_tx_traffic_ind(struct l1_model_ms *, struct msgb *msg, uint16_t arfcn, uint8_t link_id,
                          uint8_t chan_nr, uint32_t fn, uint8_t snr,
                          uint8_t signal_dbm, uint8_t num_biterr,
                          uint8_t fire_crc);
void l1ctl_tx_pm_conf(struct l1_model_ms *, struct l1ctl_pm_req *pm_req);
void l1ctl_tx_fbsb_conf(struct l1_model_ms *, uint8_t res, uint16_t arfcn);
void l1ctl_tx_ccch_mode_conf(struct l1_model_ms *, uint8_t ccch_mode);
void l1ctl_tx_tch_mode_conf(struct l1_model_ms *, uint8_t tch_mode, uint8_t audio_mode);

/* scheduler functions */
uint32_t sched_fn_ul(struct gsm_time cur_time, uint8_t chan_nr,
                     uint8_t link_id);
