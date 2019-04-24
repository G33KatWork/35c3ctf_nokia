#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include <osmocom/bb/common/networks.h>

/* GSM 03.22 Annex A */
int gsm_match_mcc(uint16_t mcc, char *imsi)
{
    uint16_t sim_mcc;

    sim_mcc = ((imsi[0] - '0') << 8)
         + ((imsi[1] - '0') << 4)
         + imsi[2] - '0';

    return (mcc == sim_mcc);
}

/* GSM 03.22 Annex A */
int gsm_match_mnc(uint16_t mcc, uint16_t mnc, char *imsi)
{
    uint16_t sim_mnc;

    /* 1. SIM-MCC = BCCH-MCC */
    if (!gsm_match_mcc(mcc, imsi))
        return 0;

    /* 2. 3rd digit of BCCH-MNC is not 0xf */
    if ((mnc & 0x00f) != 0x00f) {
        /* 3. 3 digit SIM-MNC = BCCH-MNC */
        sim_mnc = ((imsi[3] - '0') << 8)
             + ((imsi[4] - '0') << 4)
             + imsi[5] - '0';

        return (mnc == sim_mnc);
    }

    /* 4. BCCH-MCC in the range 310-316 */
    if (mcc >= 310 && mcc <= 316) {
        /* 5. 3rd diit of SIM-MNC is 0 */
        if (imsi[5] != 0)
            return 0;
    }

    /* 6. 1st 2 digits of SIM-MNC and BCCH-MNC match */
    sim_mnc = ((imsi[3] - '0') << 8)
         + ((imsi[4] - '0') << 4)
         + 0x00f;

    return (mnc == sim_mnc);
}

const char *gsm_print_mcc(uint16_t mcc)
{
    static char string[5] = "000";

    snprintf(string, 4, "%03x", mcc);
    return string;
}

const char *gsm_print_mnc(uint16_t mnc)
{
    static char string[7];

    /* invalid format: return hex value */
    if ((mnc & 0xf000)
     || (mnc & 0x0f00) > 0x0900
     || (mnc & 0x00f0) > 0x0090
     || ((mnc & 0x000f) > 0x0009 && (mnc & 0x000f) < 0x000f)) {
        snprintf(string, 6, "0x%03x", mnc);
        return string;
    }

    /* two digits */
    if ((mnc & 0x000f) == 0x000f) {
        snprintf(string, 6, "%02x", mnc >> 4);
        return string;
    }

    /* three digits */
    snprintf(string, 6, "%03x", mnc);
    return string;
}

const uint16_t gsm_input_mcc(char *string)
{
    uint16_t mcc;

    if (strlen(string) != 3)
        return GSM_INPUT_INVALID;
    if (string[0] < '0' || string [0] > '9'
     || string[1] < '0' || string [1] > '9'
     || string[2] < '0' || string [2] > '9')
        return GSM_INPUT_INVALID;

    mcc = ((string[0] - '0') << 8)
        | ((string[1] - '0') << 4)
        | ((string[2] - '0'));

    if (mcc == 0x000)
        return GSM_INPUT_INVALID;

    return mcc;
}

const uint16_t gsm_input_mnc(char *string)
{
    uint16_t mnc = 0;

    if (strlen(string) == 2) {
        if (string[0] < '0' || string [0] > '9'
         || string[1] < '0' || string [1] > '9')
            return GSM_INPUT_INVALID;

        mnc = ((string[0] - '0') << 8)
            | ((string[1] - '0') << 4)
            | 0x00f;
    } else
    if (strlen(string) == 3) {
        if (string[0] < '0' || string [0] > '9'
         || string[1] < '0' || string [1] > '9'
         || string[2] < '0' || string [2] > '9')
            return GSM_INPUT_INVALID;

        mnc = ((string[0] - '0') << 8)
            | ((string[1] - '0') << 4)
            | ((string[2] - '0'));
    }

    return mnc;
}
