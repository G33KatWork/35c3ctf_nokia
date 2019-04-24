#include "utils.h"

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

static char namebuf[255];
static char hexd_buff[4096];
static const char hex_chars[] = "0123456789abcdef";

size_t util_strlcpy(char *dst, const char *src, size_t siz)
{
    size_t ret = src ? strlen(src) : 0;

    if (siz) {
        size_t len = (ret >= siz) ? siz - 1 : ret;
        if (src)
            memcpy(dst, src, len);
        dst[len] = '\0';
    }
    return ret;
}

char *util_hexdump(const unsigned char *buf, int len)
{
    int i;
    char *cur = hexd_buff;

    hexd_buff[0] = 0;
    for (i = 0; i < len; i++) {
        const char *delimp = " ";
        int len_remain = sizeof(hexd_buff) - (cur - hexd_buff);
        if (len_remain < 3)
            break;

        *cur++ = hex_chars[buf[i] >> 4];
        *cur++ = hex_chars[buf[i] & 0xf];

        while (len_remain > 1 && *delimp) {
            *cur++ = *delimp++;
            len_remain--;
        }

        *cur = 0;
    }
    hexd_buff[sizeof(hexd_buff)-1] = 0;
    return hexd_buff;
}

const char *get_value_string(const struct value_string *vs, uint32_t val)
{
    const char *str = get_value_string_or_null(vs, val);
    if (str)
        return str;

    snprintf(namebuf, sizeof(namebuf), "unknown 0x%"PRIx32, val);
    namebuf[sizeof(namebuf) - 1] = '\0';
    return namebuf;
}

const char *get_value_string_or_null(const struct value_string *vs,
                     uint32_t val)
{
    int i;

    for (i = 0;; i++) {
        if (vs[i].value == 0 && vs[i].str == NULL)
            break;
        if (vs[i].value == val)
            return vs[i].str;
    }

    return NULL;
}

void util_sim_pin_prepare(char *pinstr)
{
    for(int i = 0; i < 8; i++)
        if(pinstr[i] == (char)0xFF)
            pinstr[i] = (char)0x00;
}

static int hexchr_to_nible(char c)
{
    if(c >= '0' && c <= '9')
        return c - '0';
    if(c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if(c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    else
        return 0;
}

void util_unhexlify(uint8_t *dst, char *src, size_t len)
{
    for(size_t i = 0; i < len; i++)
        dst[i] = (hexchr_to_nible(src[2*i]) << 4) | hexchr_to_nible(src[2*i+1]);
}

void util_hexlify(char *dst, uint8_t *src, size_t len)
{
    for(size_t i = 0; i < len; i++) {
        dst[2*i] = hex_chars[src[i] >> 4];
        dst[2*i+1] = hex_chars[src[i] & 0xF];
    }
}

uint16_t htons(uint16_t x)
{
  return __builtin_bswap16(x);
}
