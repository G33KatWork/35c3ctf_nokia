/*
 * (C) 2010 by Andreas Eversberg <jolly@eversberg.eu>
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#include <osmocom/core/bitvec.h>

#include <osmocom/bb/common/osmocom_data.h>
#include <osmocom/bb/common/networks.h>
#include <osmocom/bb/common/logging.h>
#include <osmocom/bb/common/sysinfo.h>

#define MIN(a, b) ((a < b) ? a : b)

/*
 * dumping
 */

// FIXME: move to libosmocore
char *gsm_print_arfcn(uint16_t arfcn)
{
    static char text[10];

    sprintf(text, "%d", arfcn & 1023);
    if ((arfcn & ARFCN_PCS))
        strcat(text, "(PCS)");
    else if (arfcn >= 512 && arfcn <= 885)
        strcat(text, "(DCS)");

    return text;
}

/* check if the cell 'talks' about DCS (0) or PCS (1) */
uint8_t gsm_refer_pcs(uint16_t arfcn, struct gsm48_sysinfo *s)
{
    /* If ARFCN is PCS band, the cell refers to PCS */
    if ((arfcn & ARFCN_PCS))
        return 1;

    /* If no SI1 is available, we assume DCS. Be sure to call this
     * function only if SI 1 is available. */
    if (!s->si1)
        return 0;

    /* If band indicator indicates PCS band, the cell refers to PCSThe  */
    return s->band_ind;
}

/*
 * decoding
 */

int gsm48_decode_chan_h0(struct gsm48_chan_desc *cd, uint8_t *tsc, 
    uint16_t *arfcn)
{
    *tsc = cd->h0.tsc;
    *arfcn = cd->h0.arfcn_low | (cd->h0.arfcn_high << 8);

    return 0;
}

int gsm48_decode_chan_h1(struct gsm48_chan_desc *cd, uint8_t *tsc,
    uint8_t *maio, uint8_t *hsn)
{
    *tsc = cd->h1.tsc;
    *maio = cd->h1.maio_low | (cd->h1.maio_high << 2);
    *hsn = cd->h1.hsn;

    return 0;
}

/* decode "Cell Channel Description" (10.5.2.1b) and other frequency lists */
static int decode_freq_list(struct gsm_sysinfo_freq *f, uint8_t *cd,
    uint8_t len, uint8_t mask, uint8_t frqt)
{
#if 0
    /* only Bit map 0 format for P-GSM */
    if ((cd[0] & 0xc0 & mask) != 0x00 &&
        (set->p_gsm && !set->e_gsm && !set->r_gsm && !set->dcs))
        return 0;
#endif

    return gsm48_decode_freq_list(f, cd, len, mask, frqt);
}

/* decode "Cell Selection Parameters" (10.5.2.4) */
static int gsm48_decode_cell_sel_param(struct gsm48_sysinfo *s,
    struct gsm48_cell_sel_par *cs)
{
    s->ms_txpwr_max_cch = cs->ms_txpwr_max_ccch;
    s->cell_resel_hyst_db = cs->cell_resel_hyst * 2;
    s->rxlev_acc_min_db = cs->rxlev_acc_min - 110;
    s->neci = cs->neci;
    s->acs = cs->acs;

    return 0;
}

/* decode "Cell Options (BCCH)" (10.5.2.3) */
static int gsm48_decode_cellopt_bcch(struct gsm48_sysinfo *s,
    struct gsm48_cell_options *co)
{
    s->bcch_radio_link_timeout = (co->radio_link_timeout + 1) * 4;
    s->bcch_dtx = co->dtx;
    s->bcch_pwrc = co->pwrc;

    return 0;
}

/* decode "Cell Options (SACCH)" (10.5.2.3a) */
static int gsm48_decode_cellopt_sacch(struct gsm48_sysinfo *s,
    struct gsm48_cell_options *co)
{
    s->sacch_radio_link_timeout = (co->radio_link_timeout + 1) * 4;
    s->sacch_dtx = co->dtx;
    s->sacch_pwrc = co->pwrc;

    return 0;
}

/* decode "Control Channel Description" (10.5.2.11) */
static int gsm48_decode_ccd(struct gsm48_sysinfo *s,
    struct gsm48_control_channel_descr *cc)
{
    s->ccch_conf = cc->ccch_conf;
    s->bs_ag_blks_res = cc->bs_ag_blks_res;
    s->att_allowed = cc->att;
    s->pag_mf_periods = cc->bs_pa_mfrms + 2;
    s->t3212 = cc->t3212 * 360; /* convert deci-hours to seconds */

    return 0;
}

/* decode "Mobile Allocation" (10.5.2.21) */
int gsm48_decode_mobile_alloc(struct gsm_sysinfo_freq *freq,
    uint8_t *ma, uint8_t len, uint16_t *hopping, uint8_t *hopp_len, int si4)
{
    int i, j = 0;
    uint16_t f[len << 3];

    /* not more than 64 hopping indexes allowed in IE */
    if (len > 8)
        return -EINVAL;

    /* tabula rasa */
    *hopp_len = 0;
    if (si4) {
        for (i = 0; i < 1024; i++)
            freq[i].mask &= ~FREQ_TYPE_HOPP;
    }

    /* generating list of all frequencies (1..1023,0) */
    for (i = 1; i <= 1024; i++) {
        if ((freq[i & 1023].mask & FREQ_TYPE_SERV)) {
            LOGP(DRR, LOGL_INFO, "Serving cell ARFCN #%d: %d\n",
                j, i & 1023);
            f[j++] = i & 1023;
            if (j == (len << 3))
                break;
        }
    }

    /* fill hopping table with frequency index given by IE
     * and set hopping type bits
     */
    for (i = 0; i < (len << 3); i++) {
        /* if bit is set, this frequency index is used for hopping */
        if ((ma[len - 1 - (i >> 3)] & (1 << (i & 7)))) {
            LOGP(DRR, LOGL_INFO, "Hopping ARFCN: %d (bit %d)\n",
                i, f[i]);
            /* index higher than entries in list ? */
            if (i >= j) {
                LOGP(DRR, LOGL_NOTICE, "Mobile Allocation "
                    "hopping index %d exceeds maximum "
                    "number of cell frequencies. (%d)\n",
                    i + 1, j);
                break;
            }
            hopping[(*hopp_len)++] = f[i];
            if (si4)
                freq[f[i]].mask |= FREQ_TYPE_HOPP;
        }
    }

    return 0;
}

/* Rach Control decode tables */
static uint8_t gsm48_max_retrans[4] = {
    1, 2, 4, 7
};
static uint8_t gsm48_tx_integer[16] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 20, 25, 32, 50
};

/* decode "RACH Control Parameter" (10.5.2.29) */
static int gsm48_decode_rach_ctl_param(struct gsm48_sysinfo *s,
    struct gsm48_rach_control *rc)
{
    s->reest_denied = rc->re;
    s->cell_barr = rc->cell_bar;
    s->tx_integer = gsm48_tx_integer[rc->tx_integer];
    s->max_retrans = gsm48_max_retrans[rc->max_trans];
    s->class_barr = (rc->t2 << 8) | rc->t3;

    return 0;
}
static int gsm48_decode_rach_ctl_neigh(struct gsm48_sysinfo *s,
    struct gsm48_rach_control *rc)
{
    s->nb_reest_denied = rc->re;
    s->nb_cell_barr = rc->cell_bar;
    s->nb_tx_integer = gsm48_tx_integer[rc->tx_integer];
    s->nb_max_retrans = gsm48_max_retrans[rc->max_trans];
    s->nb_class_barr = (rc->t2 << 8) | rc->t3;

    return 0;
}

/* decode "SI 1 Rest Octets" (10.5.2.32) */
static int gsm48_decode_si1_rest(struct gsm48_sysinfo *s, uint8_t *si,
    uint8_t len)
{
    struct bitvec bv;

    memset(&bv, 0, sizeof(bv));
    bv.data_len = len;
    bv.data = si;

    /* Optional Selection Parameters */
    if (bitvec_get_bit_high(&bv) == H) {
        s->nch = 1;
        s->nch_position = bitvec_get_uint(&bv, 5);
    } else
        s->nch = 0;
    if (bitvec_get_bit_high(&bv) == H)
        s->band_ind = 1;
    else
        s->band_ind = 0;

    return 0;
}

/* decode "SI 3 Rest Octets" (10.5.2.34) */
static int gsm48_decode_si3_rest(struct gsm48_sysinfo *s, uint8_t *si,
    uint8_t len)
{
    struct bitvec bv;

    memset(&bv, 0, sizeof(bv));
    bv.data_len = len;
    bv.data = si;

    /* Optional Selection Parameters */
    if (bitvec_get_bit_high(&bv) == H) {
        s->sp = 1;
        s->sp_cbq = bitvec_get_uint(&bv, 1);
        s->sp_cro = bitvec_get_uint(&bv, 6);
        s->sp_to = bitvec_get_uint(&bv, 3);
        s->sp_pt = bitvec_get_uint(&bv, 5);
    } else
        s->sp = 0;
    /* Optional Power Offset */
    if (bitvec_get_bit_high(&bv) == H) {
        s->po = 1;
        s->po_value = bitvec_get_uint(&bv, 2);
    } else
        s->po = 0;
    /* System Onformation 2ter Indicator */
    if (bitvec_get_bit_high(&bv) == H)
        s->si2ter_ind = 1;
    else
        s->si2ter_ind = 0;
    /* Early Classark Sending Control */
    if (bitvec_get_bit_high(&bv) == H)
        s->ecsm = 1;
    else
        s->ecsm = 0;
    /* Scheduling if and where */
    if (bitvec_get_bit_high(&bv) == H) {
        s->sched = 1;
        s->sched_where = bitvec_get_uint(&bv, 3);
    } else
        s->sched = 0;
    /* GPRS Indicator */
    if (bitvec_get_bit_high(&bv) == H) {
        s->gprs = 1;
        s->gprs_ra_colour = bitvec_get_uint(&bv, 3);
        s->gprs_si13_pos = bitvec_get_uint(&bv, 1);
    } else
        s->gprs = 0;

    return 0;
}

/* decode "SI 4 Rest Octets" (10.5.2.35) */
static int gsm48_decode_si4_rest(struct gsm48_sysinfo *s, uint8_t *si,
    uint8_t len)
{
    struct bitvec bv;

    memset(&bv, 0, sizeof(bv));
    bv.data_len = len;
    bv.data = si;

    /* Optional Selection Parameters */
    if (bitvec_get_bit_high(&bv) == H) {
        s->sp = 1;
        s->sp_cbq = bitvec_get_uint(&bv, 1);
        s->sp_cro = bitvec_get_uint(&bv, 6);
        s->sp_to = bitvec_get_uint(&bv, 3);
        s->sp_pt = bitvec_get_uint(&bv, 5);
    } else
        s->sp = 0;
    /* Optional Power Offset */
    if (bitvec_get_bit_high(&bv) == H) {
        s->po = 1;
        s->po_value = bitvec_get_uint(&bv, 3);
    } else
        s->po = 0;
    /* GPRS Indicator */
    if (bitvec_get_bit_high(&bv) == H) {
        s->gprs = 1;
        s->gprs_ra_colour = bitvec_get_uint(&bv, 3);
        s->gprs_si13_pos = bitvec_get_uint(&bv, 1);
    } else
        s->gprs = 0;
    // todo: more rest octet bits

    return 0;
}

/* decode "SI 6 Rest Octets" (10.5.2.35a) */
static int gsm48_decode_si6_rest(struct gsm48_sysinfo *s, uint8_t *si,
    uint8_t len)
{
    return 0;
}

int gsm48_decode_sysinfo1(struct gsm48_sysinfo *s,
        struct gsm48_system_information_type_1 *si, int len)
{
    int payload_len = len - sizeof(*si);

    memcpy(s->si1_msg, si, MIN(len, sizeof(s->si1_msg)));

    /* Cell Channel Description */
    decode_freq_list(s->freq, si->cell_channel_description,
        sizeof(si->cell_channel_description), 0xce, FREQ_TYPE_SERV);
    /* RACH Control Parameter */
    gsm48_decode_rach_ctl_param(s, &si->rach_control);
    /* SI 1 Rest Octets */
    if (payload_len)
        gsm48_decode_si1_rest(s, si->rest_octets, payload_len);

    s->si1 = 1;

    if (s->si4) {
        LOGP(DRR, LOGL_NOTICE, "Now updating previously received "
            "SYSTEM INFORMATION 4\n");
        gsm48_decode_sysinfo4(s,
            (struct gsm48_system_information_type_4 *) s->si4_msg,
            sizeof(s->si4_msg));
    }

    return 0;
}

int gsm48_decode_sysinfo2(struct gsm48_sysinfo *s,
        struct gsm48_system_information_type_2 *si, int len)
{
    memcpy(s->si2_msg, si, MIN(len, sizeof(s->si2_msg)));

    /* Neighbor Cell Description */
    s->nb_ext_ind_si2 = (si->bcch_frequency_list[0] >> 6) & 1;
    s->nb_ba_ind_si2 = (si->bcch_frequency_list[0] >> 5) & 1;
    decode_freq_list(s->freq, si->bcch_frequency_list,
        sizeof(si->bcch_frequency_list), 0xce, FREQ_TYPE_NCELL_2);
    /* NCC Permitted */
    s->nb_ncc_permitted_si2 = si->ncc_permitted;
    /* RACH Control Parameter */
    gsm48_decode_rach_ctl_neigh(s, &si->rach_control);

    s->si2 = 1;

    return 0;
}

int gsm48_decode_sysinfo2bis(struct gsm48_sysinfo *s,
        struct gsm48_system_information_type_2bis *si, int len)
{
    memcpy(s->si2b_msg, si, MIN(len, sizeof(s->si2b_msg)));

    /* Neighbor Cell Description */
    s->nb_ext_ind_si2bis = (si->bcch_frequency_list[0] >> 6) & 1;
    s->nb_ba_ind_si2bis = (si->bcch_frequency_list[0] >> 5) & 1;
    decode_freq_list(s->freq, si->bcch_frequency_list,
        sizeof(si->bcch_frequency_list), 0xce, FREQ_TYPE_NCELL_2bis);
    /* RACH Control Parameter */
    gsm48_decode_rach_ctl_neigh(s, &si->rach_control);

    s->si2bis = 1;

    return 0;
}

int gsm48_decode_sysinfo2ter(struct gsm48_sysinfo *s,
        struct gsm48_system_information_type_2ter *si, int len)
{
    memcpy(s->si2t_msg, si, MIN(len, sizeof(s->si2t_msg)));

    /* Neighbor Cell Description 2 */
    s->nb_multi_rep_si2ter = (si->ext_bcch_frequency_list[0] >> 6) & 3;
    s->nb_ba_ind_si2ter = (si->ext_bcch_frequency_list[0] >> 5) & 1;
    decode_freq_list(s->freq, si->ext_bcch_frequency_list,
        sizeof(si->ext_bcch_frequency_list), 0x8e,
            FREQ_TYPE_NCELL_2ter);

    s->si2ter = 1;

    return 0;
}

int gsm48_decode_sysinfo3(struct gsm48_sysinfo *s,
        struct gsm48_system_information_type_3 *si, int len)
{
    int payload_len = len - sizeof(*si);

    memcpy(s->si3_msg, si, MIN(len, sizeof(s->si3_msg)));

    /* Cell Identity */
    s->cell_id = ntohs(si->cell_identity);
    /* LAI */
    gsm48_decode_lai_hex(&si->lai, &s->mcc, &s->mnc, &s->lac);
    /* Control Channel Description */
    gsm48_decode_ccd(s, &si->control_channel_desc);
    /* Cell Options (BCCH) */
    gsm48_decode_cellopt_bcch(s, &si->cell_options);
    /* Cell Selection Parameters */
    gsm48_decode_cell_sel_param(s, &si->cell_sel_par);
    /* RACH Control Parameter */
    gsm48_decode_rach_ctl_param(s, &si->rach_control);
    /* SI 3 Rest Octets */
    if (payload_len >= 4)
        gsm48_decode_si3_rest(s, si->rest_octets, payload_len);

    LOGP(DRR, LOGL_INFO, "New SYSTEM INFORMATION 3 (mcc %s mnc %s "
        "lac 0x%04x)\n", gsm_print_mcc(s->mcc),
        gsm_print_mnc(s->mnc), s->lac);

    s->si3 = 1;

    return 0;
}

int gsm48_decode_sysinfo4(struct gsm48_sysinfo *s,
        struct gsm48_system_information_type_4 *si, int len)
{
    int payload_len = len - sizeof(*si);

    uint8_t *data = si->data;
    struct gsm48_chan_desc *cd;

    memcpy(s->si4_msg, si, MIN(len, sizeof(s->si4_msg)));

    /* LAI */
    gsm48_decode_lai_hex(&si->lai, &s->mcc, &s->mnc, &s->lac);
    /* Cell Selection Parameters */
    gsm48_decode_cell_sel_param(s, &si->cell_sel_par);
    /* RACH Control Parameter */
    gsm48_decode_rach_ctl_param(s, &si->rach_control);

    /* CBCH Channel Description */
    if (payload_len >= 1 && data[0] == GSM48_IE_CBCH_CHAN_DESC) {
        if (payload_len < 4) {
short_read:
            LOGP(DRR, LOGL_NOTICE, "Short read!\n");
            return -EIO;
        }
        cd = (struct gsm48_chan_desc *) (data + 1);
        s->chan_nr = cd->chan_nr;
        if (cd->h0.h) {
            s->h = 1;
            gsm48_decode_chan_h1(cd, &s->tsc, &s->maio, &s->hsn);
        } else {
            s->h = 0;
            gsm48_decode_chan_h0(cd, &s->tsc, &s->arfcn);
        }
        payload_len -= 4;
        data += 4;
    }
    /* CBCH Mobile Allocation */
    if (payload_len >= 1 && data[0] == GSM48_IE_CBCH_MOB_AL) {
        if (payload_len < 1 || payload_len < 2 + data[1])
            goto short_read;
        if (!s->si1) {
            LOGP(DRR, LOGL_NOTICE, "Ignoring CBCH allocation of "
                "SYSTEM INFORMATION 4 until SI 1 is "
                "received.\n");
        } else {
            gsm48_decode_mobile_alloc(s->freq, data + 2, data[1],
                s->hopping, &s->hopp_len, 1);
        }
        payload_len -= 2 + data[1];
        data += 2 + data[1];
    }
    /* SI 4 Rest Octets */
    if (payload_len > 0)
        gsm48_decode_si4_rest(s, data, payload_len);

    s->si4 = 1;

    return 0;
}

int gsm48_decode_sysinfo5(struct gsm48_sysinfo *s,
        struct gsm48_system_information_type_5 *si, int len)
{
    memcpy(s->si5_msg, si, MIN(len, sizeof(s->si5_msg)));

    /* Neighbor Cell Description */
    s->nb_ext_ind_si5 = (si->bcch_frequency_list[0] >> 6) & 1;
    s->nb_ba_ind_si5 = (si->bcch_frequency_list[0] >> 5) & 1;
    decode_freq_list(s->freq, si->bcch_frequency_list,
        sizeof(si->bcch_frequency_list), 0xce, FREQ_TYPE_REP_5);

    s->si5 = 1;

    return 0;
}

int gsm48_decode_sysinfo5bis(struct gsm48_sysinfo *s,
        struct gsm48_system_information_type_5bis *si, int len)
{
    memcpy(s->si5b_msg, si, MIN(len, sizeof(s->si5b_msg)));

    /* Neighbor Cell Description */
    s->nb_ext_ind_si5bis = (si->bcch_frequency_list[0] >> 6) & 1;
    s->nb_ba_ind_si5bis = (si->bcch_frequency_list[0] >> 5) & 1;
    decode_freq_list(s->freq, si->bcch_frequency_list,
        sizeof(si->bcch_frequency_list), 0xce, FREQ_TYPE_REP_5bis);

    s->si5bis = 1;

    return 0;
}

int gsm48_decode_sysinfo5ter(struct gsm48_sysinfo *s,
        struct gsm48_system_information_type_5ter *si, int len)
{
    memcpy(s->si5t_msg, si, MIN(len, sizeof(s->si5t_msg)));

    /* Neighbor Cell Description */
    s->nb_multi_rep_si5ter = (si->bcch_frequency_list[0] >> 6) & 3;
    s->nb_ba_ind_si5ter = (si->bcch_frequency_list[0] >> 5) & 1;
    decode_freq_list(s->freq, si->bcch_frequency_list,
        sizeof(si->bcch_frequency_list), 0x8e, FREQ_TYPE_REP_5ter);

    s->si5ter = 1;

    return 0;
}

int gsm48_decode_sysinfo6(struct gsm48_sysinfo *s,
        struct gsm48_system_information_type_6 *si, int len)
{
    int payload_len = len - sizeof(*si);

    memcpy(s->si6_msg, si, MIN(len, sizeof(s->si6_msg)));

    /* Cell Identity */
    if (s->si6 && s->cell_id != ntohs(si->cell_identity))
        LOGP(DRR, LOGL_INFO, "Cell ID on SI 6 differs from previous "
            "read.\n");
    s->cell_id = ntohs(si->cell_identity);
    /* LAI */
    gsm48_decode_lai_hex(&si->lai, &s->mcc, &s->mnc, &s->lac);
    /* Cell Options (SACCH) */
    gsm48_decode_cellopt_sacch(s, &si->cell_options);
    /* NCC Permitted */
    s->nb_ncc_permitted_si6 = si->ncc_permitted;
    /* SI 6 Rest Octets */
    if (payload_len >= 4)
        gsm48_decode_si6_rest(s, si->rest_octets, payload_len);

    s->si6 = 1;

    return 0;
}

int gsm48_encode_lai_hex(struct gsm48_loc_area_id *lai, uint16_t mcc,
    uint16_t mnc, uint16_t lac)
{
    lai->digits[0] = (mcc >> 8) | (mcc & 0xf0);
    lai->digits[1] = (mcc & 0x0f) | (mnc << 4);
    lai->digits[2] = (mnc >> 8) | (mnc & 0xf0);
    lai->lac = htons(lac);

    return 0;
}

 int gsm48_decode_lai_hex(struct gsm48_loc_area_id *lai, uint16_t *mcc,
        uint16_t *mnc, uint16_t *lac)
{
    *mcc = ((lai->digits[0] & 0x0f) << 8)
        | (lai->digits[0] & 0xf0)
        | (lai->digits[1] & 0x0f);
    *mnc = ((lai->digits[2] & 0x0f) << 8)
        | (lai->digits[2] & 0xf0)
        | ((lai->digits[1] & 0xf0) >> 4);
    *lac = ntohs(lai->lac);
                    
    return 0;
}
