#ifndef _SIM_TRUSTZONE_H_
#define _SIM_TRUSTZONE_H_

#include <stdint.h>

#include <osmocom/bb/common/osmocom_data.h>

int trustzonecard_open(struct osmocom_ms *ms);
int trustzonecard_close(struct osmocom_ms *ms);

int trustzonecard_send_apdu(struct osmocom_ms *ms, uint8_t *data, uint16_t length);

#endif
