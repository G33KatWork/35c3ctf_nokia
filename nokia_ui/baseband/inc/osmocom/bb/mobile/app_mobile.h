#ifndef APP_MOBILE_H
#define APP_MOBILE_H

#include <stdbool.h>

struct osmocom_ms;
struct msgb;

void app_init(void);
void app_shutdown(void);
int app_dowork(void);


struct osmocom_ms *mobile_new(char *name, int (*ui_recv_socket)(struct osmocom_ms *ms, struct msgb *msg));
int mobile_delete(struct osmocom_ms *ms, int force);
int mobile_work(struct osmocom_ms *ms);

int mobile_start(struct osmocom_ms *ms);
int mobile_stop(struct osmocom_ms *ms, int force);

/* Internal code. Don't call directly */
int mobile_exit(struct osmocom_ms *ms, int force);
#endif
