#include "sms_receive.h"

#include <string.h>

#include <osmocom/gsm/gsm48_ie.h>
#include <osmocom/gsm/protocol/gsm_04_11.h>
#include <osmocom/gsm/gsm0411_utils.h>
#include <osmocom/gsm/gsm_utils.h>

#include "db.h"

struct gsm_sms {
    uint8_t ud_hdr_ind;
    uint8_t protocol_id;
    uint8_t data_coding_scheme;
    char address[20+1]; /* DA LV is 12 bytes max, i.e. 10 bytes
                         * BCD == 20 bytes string */
    time_t time;
    uint8_t user_data_len;
    uint8_t user_data[163];
};

struct sms_part {
    struct gsm_sms *sms;
    uint8_t part_num;
    struct llist_head entry;
};

struct concatenated_sms {
    uint8_t ref_num;
    uint8_t total_parts;
    struct llist_head parts;
    struct llist_head entry;
};

static LLIST_HEAD(pending_csms);

static int sms_receive_parse_tpdu(uint8_t *tpdu, struct gsm_sms *sms)
{
    uint8_t *smsp = tpdu;

    uint8_t sms_mti;
    uint8_t oa_len_bytes;
    uint8_t address_lv[12]; /* according to 03.40 / 9.1.2.5 */
    unsigned int sms_alphabet;


    sms_mti = *smsp & 0x03;
    if(sms_mti != GSM340_SMS_DELIVER_SC2MS)
        return 1;

    sms->ud_hdr_ind = (*smsp & 0x40);
    smsp++;


    /* length in bytes of the originate address */
    oa_len_bytes = 2 + *smsp/2 + *smsp%2;
    if (oa_len_bytes > 12) {
        printf("SMS Originate Address > 12 bytes ?!?\n");
        return 1;
    }

    memset(address_lv, 0, sizeof(address_lv));
    memcpy(address_lv, smsp, oa_len_bytes);

    /* mangle first byte to reflect length in bytes, not digits */
    address_lv[0] = oa_len_bytes - 1;

    /* convert to real number */
    if (((smsp[1] & 0x70) >> 4) == 1)
        strcpy(sms->address, "+");
    else if (((smsp[1] & 0x70) >> 4) == 2)
        strcpy(sms->address, "0");
    else
        sms->address[0] = '\0';
    
    gsm48_decode_bcd_number(sms->address + strlen(sms->address), sizeof(sms->address) - strlen(sms->address), address_lv, 1);
    smsp += oa_len_bytes;


    sms->protocol_id = *smsp++;
    sms->data_coding_scheme = *smsp++;


    sms_alphabet = gsm338_get_sms_alphabet(sms->data_coding_scheme);
    if(sms_alphabet == 0xffffffff)
        return 1;


    /* get timestamp */
    sms->time = gsm340_scts(smsp);
    smsp += 7;


    /* user data */
    sms->user_data_len = *smsp++;
    if(sms->user_data_len)
        memcpy(sms->user_data, smsp, sms->user_data_len);


    return 0;
}

static int sms_parse_concat_sms_udh(struct gsm_sms *sms, uint8_t *ref_num, uint8_t *total_parts, uint8_t *part_num)
{
    uint8_t udhl = sms->user_data[0];
    uint8_t info_element = sms->user_data[1]; //Information element identifier: 0 = concatenated SMS

    switch(info_element) {
        case 0:
            if(udhl != 5) {
                printf("Concatenated SMS UDH with length != 5?\n");
                return 1;
            }

            uint8_t header_len = sms->user_data[2];
            if(header_len != 3) {
                printf("Concatenated SMS UDH with header length != 3?\n");
                return 1;
            }

            uint8_t csms_total_parts = sms->user_data[4];
            if(total_parts == 0 || csms_total_parts > 3) {
                printf("Too many parts\n");
                return 1;
            }

            *ref_num = sms->user_data[3];
            *total_parts = csms_total_parts;
            *part_num = sms->user_data[5];

            break;
        default:
            printf("Got unknown information element in UDH: 0x%02x\n", info_element);
            return 1;
    }

    return 0;
}

static struct concatenated_sms *find_csms(uint8_t ref_num)
{
    struct concatenated_sms *csms;

    llist_for_each_entry(csms, &pending_csms, entry) {
        if(csms->ref_num == ref_num)
            return csms;
    }

    return NULL;
}

static void sms_reassemble_concat_and_deliver(struct nokia_ui *ui, struct concatenated_sms *csms)
{
    char payload[460] = {0};
    char address[20+1] = {0};
    struct sms_part *part, *part2;

    llist_for_each_entry_safe(part, part2, &csms->parts, entry) {
        switch (gsm338_get_sms_alphabet(part->sms->data_coding_scheme)) {
            case DCS_7BIT_DEFAULT:
                // BUG 35C3CTF: part number isn't bounds checked, OOB array write
                gsm_7bit_decode_n_hdr(&payload[(part->part_num-1)*153], 154, part->sms->user_data, part->sms->user_data_len, part->sms->ud_hdr_ind);
                break;

            default:
                printf("Unknown data coding scheme in sms. Copying raw data after UDH\n");
            case DCS_UCS2:
            case DCS_8BIT_DATA:
                printf("8 bit encoding\n");
                //6 bytes are to skip UDH header for CSMS
                // BUG 35C3CTF: part number isn't bounds checked, OOB array write
                memcpy(&payload[(part->part_num-1)*(140-6)], &part->sms->user_data[6], (part->sms->user_data_len-6) > (140-6) ? (140-6) : (part->sms->user_data_len-6));
                break;
        }

        strncpy(address, part->sms->address, sizeof(address)-1);

        llist_del(&part->entry);
        talloc_free(part->sms);
        talloc_free(part);
    }

    db_add_message_entry(address, payload);
    ui->ui_entity.unread_sms++;
}

static void handle_pending_csms(struct nokia_ui *ui)
{
    struct concatenated_sms *csms, *csms2;

    llist_for_each_entry_safe(csms, csms2, &pending_csms, entry) {
        uint8_t cur_num_parts = llist_count(&csms->parts);
        printf("CSMS 0x%02x has 0x%02x parts. Current part count: 0x%02x\n", csms->ref_num, csms->total_parts, cur_num_parts);
        if(cur_num_parts == csms->total_parts) {
            sms_reassemble_concat_and_deliver(ui, csms);
            llist_del(&csms->entry);
            talloc_free(csms);
        }
    }
}

void sms_receive(struct nokia_ui *ui, uint8_t tpdu_len, uint8_t *tpdu)
{
    struct gsm_sms *sms = talloc_zero(ui, struct gsm_sms);

    if(sms_receive_parse_tpdu(tpdu, sms)) {
        talloc_free(sms);
        return;
    }

    if(!sms->ud_hdr_ind) {
        printf("Received normal SMS\n");
        
        char sms_text[161] = {0};

        switch (gsm338_get_sms_alphabet(sms->data_coding_scheme)) {
            case DCS_7BIT_DEFAULT:
                gsm_7bit_decode_n_hdr(sms_text, sizeof(sms_text)-1, sms->user_data, sms->user_data_len, sms->ud_hdr_ind);
                break;

            default:
                printf("Unknown data coding scheme in sms. Copying raw data\n");
            case DCS_UCS2:
            case DCS_8BIT_DATA:
                memcpy(sms_text, sms->user_data, sms->user_data_len > 140 ? 140 : sms->user_data_len);
                break;
        }

        db_add_message_entry(sms->address, sms_text);
        ui->ui_entity.unread_sms++;

        talloc_free(sms);
    } else {
        printf("Received SMS with UDH\n");

        uint8_t ref_num;
        uint8_t total_parts;
        uint8_t part_num;

        if(sms_parse_concat_sms_udh(sms, &ref_num, &total_parts, &part_num)) {
            printf("Received SMS with malformed/unknown UDH. Discarding...\n");
            talloc_free(sms);
        } else {
            struct concatenated_sms* csms = find_csms(ref_num);
            if(!csms) {
                csms = talloc_zero(ui, struct concatenated_sms);
                csms->ref_num = ref_num;
                csms->total_parts = total_parts;
                INIT_LLIST_HEAD(&csms->parts);
                llist_add_tail(&csms->entry, &pending_csms);
            }

            struct sms_part *sms_part = talloc_zero(ui, struct sms_part);
            sms_part->sms = sms;
            sms_part->part_num = part_num;
            llist_add_tail(&sms_part->entry, &csms->parts);
        }

        handle_pending_csms(ui);
    }
}
