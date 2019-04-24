#!/usr/bin/env python2

import sys
import os

from PIL import Image

if len(sys.argv) != 5:
	print "Usage: {0} <configfile> <imagedirectory> <cfile> <hfile>".format(sys.argv[0])
	sys.exit(1)

c_tmpl = """#include "images.h"
{0}
"""

h_tmpl = """#ifndef _IMAGES_GENERATED_H_
#define _IMAGES_GENERATED_H_
#include "images.h"
{0}
#endif
"""

c_code = ""
h_code = ""

entries = []

config = open(sys.argv[1], "r")
for line in config:
	entry = line.strip().split("\t")
	assert(len(entry) == 2)
	entries.append((os.path.join(sys.argv[2], entry[0]), entry[1]))

for (image, cname) in entries:
	im = Image.open(image)
	(width, height) = im.size

	data = []

	curData = 0
	bitPos = 0

	for y in range(height):
		for x in range(width):
			curData = curData | (im.getpixel((x,y)) << bitPos)
			if bitPos == 7:
				data.append(curData)
				curData = 0
				bitPos = 0
			else:
				bitPos += 1

	#append last byte which isn't completely filled
	if bitPos != 0:
		data.append(curData)

	struct_c_code = """
const struct image {0} = {{
	.width = {1},
	.height = {2},
	.datalen = {3},
	.data = {{
		{4}
	}}
}};

	""".format(cname, width, height, len(data), ", ".join([hex(x) for x in data]))
	c_code += struct_c_code

	struct_h_code = """
extern const struct image {0};
	""".format(cname);
	h_code += struct_h_code

open(sys.argv[3], "w").write(c_tmpl.format(c_code))
open(sys.argv[4], "w").write(h_tmpl.format(h_code))
