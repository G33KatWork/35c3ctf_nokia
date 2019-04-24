#ifndef _SMS_H_
#define _SMS_H_

#include <stdint.h>
#include <time.h>

#include "nokia_ui.h"

void sms_send(struct nokia_ui *ui, const char* recipient, const char* text);
void sms_notify_job_completion(struct nokia_ui *ui, uint8_t msg_ref, int cause);
void sms_process_jobs(struct nokia_ui *ui);

#endif
