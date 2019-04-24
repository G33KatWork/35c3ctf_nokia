#ifndef _SIM_SOCKET_H_
#define _SIM_SOCKET_H_

#include <stdint.h>

#include <osmocom/bb/common/osmocom_data.h>
#include <osmocom/core/write_queue.h>

int socketcard_open(struct osmocom_ms *ms);
int socketcard_close(struct osmocom_ms *ms);

int socketcard_send_apdu(struct osmocom_ms *ms, uint8_t *data, uint16_t length);

#endif
