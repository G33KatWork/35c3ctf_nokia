#ifndef _UTILS_H_
#define _UTILS_H_

#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct value_string {
    unsigned int value; /*!< numeric value */
    const char *str;    /*!< human-readable string */
};

size_t util_strlcpy(char *dst, const char *src, size_t siz);
char *util_hexdump(const unsigned char *buf, int len);

const char *get_value_string(const struct value_string *vs, uint32_t val);
const char *get_value_string_or_null(const struct value_string *vs, uint32_t val);

void util_sim_pin_prepare(char *pinstr);

void util_unhexlify(uint8_t *dst, char *src, size_t len);
void util_hexlify(char *dst, uint8_t *src, size_t len);

uint16_t htons (uint16_t x);

#endif
