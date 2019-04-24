#ifndef _WORDWRAP_H_
#define _WORDWRAP_H_

#include <stdbool.h>
#include <stdint.h>
#include <fonts.h>

#include "nokia_ui.h"

typedef _Bool (*line_callback_t)(struct nokia_ui *ui, char* line, uint16_t count, void *state);
void wordwrap(struct nokia_ui *ui, const struct image* font[], int16_t width, char* text, line_callback_t callback, void *state);

#endif
