#ifndef _IMAGES_H_
#define _IMAGES_H_

#include <stdint.h>
#include <stdbool.h>

struct image {
	uint8_t width;
	uint8_t height;
	uint16_t datalen;
	uint8_t data[];
};

#endif
