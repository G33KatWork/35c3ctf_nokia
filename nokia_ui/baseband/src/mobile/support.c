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

#include <osmocom/bb/common/osmocom_data.h>

void gsm_support_init(struct osmocom_ms *ms)
{
    struct gsm_support *sup = &ms->support;

    memset(sup, 0, sizeof(*sup));
    sup->ms = ms;

    /* controlled early classmark sending */
    sup->es_ind = 1; /* yes */
    /* revision level */
    sup->rev_lev = 1; /* phase 2 mobile station */
    /* support of VGCS */
    sup->vgcs = 0; /* no */
    /* support of VBS */
    sup->vbs = 0; /* no */
    /* support of SMS */
    sup->sms_ptp = 1; /* no */
    /* screening indicator */
    sup->ss_ind = 1; /* phase 2 error handling */
    /* pseudo synchronised capability */
    sup->ps_cap = 0; /* no */
    /* CM service prompt */
    sup->cmsp = 0; /* no */
    /* solsa support */
    sup->solsa = 0; /* no */
    /* location service support */
    sup->lcsva = 0; /* no */
    sup->loc_serv = 0; /* no */
    /* cipher support */
    sup->a5_1 = 1;
    sup->a5_2 = 1;
    sup->a5_3 = 0;
    sup->a5_4 = 0;
    sup->a5_5 = 0;
    sup->a5_6 = 0;
    sup->a5_7 = 0;
    /* radio support */
    sup->p_gsm = 1; /* P-GSM */
    sup->e_gsm = 1; /* E-GSM */
    sup->r_gsm = 1; /* R-GSM */
    sup->dcs = 1;
    sup->gsm_850 = 1;
    sup->pcs = 1;
    sup->gsm_480 = 0;
    sup->gsm_450 = 0;
    /* rf power capability */
    sup->class_900 = 4; /* CLASS 4: Handheld 2W */
    sup->class_850 = 4;
    sup->class_400 = 4;
    sup->class_dcs = 1; /* CLASS 1: Handheld 1W */
    sup->class_pcs = 1;
    /* multi slot support */
    sup->ms_sup = 0; /* no */
    /* ucs2 treatment */
    sup->ucs2_treat = 0; /* default */
    /* support extended measurements */
    sup->ext_meas = 0; /* no */
    /* support switched measurement capability */
    sup->meas_cap = 0; /* no */
    //sup->sms_val = ;
    //sup->sm_val = ;

    /* radio */
    sup->ch_cap = GSM_CAP_SDCCH_TCHF_TCHH;
    sup->min_rxlev_dbm = -106; // TODO
    sup->sync_to = 6; /* how long to wait sync (0.9 s) */
    sup->scan_to = 4; /* how long to wait for all sysinfos (>=4 s) */
    sup->dsc_max = 90; /* the specs defines 90 */

    /* codec */
    sup->full_v1 = 1;
    sup->full_v2 = 1;
    sup->full_v3 = 0;
    sup->half_v1 = 1;
    sup->half_v3 = 0;
}

/* (3.2.1) maximum channels to scan within each band */
struct gsm_support_scan_max gsm_sup_smax[] = {
    { 259, 293, 15, 0 }, /* GSM 450 */
    { 306, 340, 15, 0 }, /* GSM 480 */
    { 438, 511, 25, 0 },
    { 128, 251, 30, 0 }, /* GSM 850 */
    { 955, 124, 30, 0 }, /* P,E,R GSM */
    { 512, 885, 40, 0 }, /* DCS 1800 */
    { 1024, 1322, 40, 0 }, /* PCS 1900 */
    { 0, 0, 0, 0 }
};

#define SUP_SET(item) \
    ((sup->item) ? ((set->item) ? "yes" : "disabled") : "no")
