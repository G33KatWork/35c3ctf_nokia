#!/usr/bin/env python2

import sys
import os

from PIL import Image

if len(sys.argv) != 5:
	print "Usage: {0} <configfile> <fontdirectory> <cfile> <hfile>".format(sys.argv[0])
	sys.exit(1)

c_tmpl = """#include "fonts.h"
{0}

const struct image* font_{1}[] = {{
	{2}
}};
"""

h_tmpl = """#ifndef _FONTS_{1}_GENERATED_H_
#define _FONTS_{1}_GENERATED_H_
#include "fonts.h"
{0}

extern const struct image* font_{1}[];
#endif
"""

c_code = ""
h_code = ""
c_font_code = ""

linenum = 0
characters = []
defined_characters = []

font_bmp_file = ""
char_height = 0
unknown_char_xpos = 0
unknown_char_width = 0

config = open(sys.argv[1], "r")
for line in config:
	entry = line.strip().split("\t")
	if linenum == 0:
		font_bmp_file = os.path.join(sys.argv[2], entry[0])
		char_height = int(entry[1])
		first_line = False
	elif linenum == 1:
		unknown_char_xpos = int(entry[0])
		unknown_char_width = int(entry[1])
	else:
		characters.append((int(entry[0], 16), int(entry[1])))
		defined_characters.append(int(entry[0], 16))

	linenum += 1

im = Image.open(font_bmp_file)
(width, height) = im.size

curxpos = 0

for (char, width) in characters:
	data = []

	curData = 0
	bitPos = 0

	charImage = im.transform((width, char_height), Image.EXTENT, (curxpos, 0, curxpos+width, char_height))

	for y in range(char_height):
		for x in range(width):
			curData = curData | (charImage.getpixel((x,y)) << bitPos)
			if bitPos == 7:
				data.append(curData)
				curData = 0
				bitPos = 0
			else:
				bitPos += 1

	#append last byte which isn't completely filled
	if bitPos != 0:
		data.append(curData)

	curxpos += width

	struct_c_code = """
const struct image char_{0}_{1:x} = {{
	.width = {2},
	.height = {3},
	.datalen = {4},
	.data = {{
		{5}
	}}
}};
	""".format(os.path.splitext(os.path.basename(font_bmp_file))[0], char, width, char_height, len(data), ", ".join([hex(x) for x in data]))
	c_code += struct_c_code

	struct_h_code = """
extern const struct image char_{0}_{1:x};
	""".format(os.path.splitext(os.path.basename(font_bmp_file))[0], char);
	h_code += struct_h_code

	struct_c_font_code = """[0x{1:x}] = &char_{0}_{1:x},
""".format(os.path.splitext(os.path.basename(font_bmp_file))[0], char);
	c_font_code += struct_c_font_code

#undefined character image
#always the last char in bitmap
charImage = im.transform((unknown_char_width, char_height), Image.EXTENT, (unknown_char_xpos, 0, unknown_char_xpos+unknown_char_width, char_height))
data = []
for y in range(char_height):
	for x in range(unknown_char_width):
		curData = curData | (charImage.getpixel((x,y)) << bitPos)
		if bitPos == 7:
			data.append(curData)
			curData = 0
			bitPos = 0
		else:
			bitPos += 1

if bitPos != 0:
	data.append(curData)

struct_c_code = """
const struct image char_{0}_unknown = {{
	.width = {1},
	.height = {2},
	.datalen = {3},
	.data = {{
		{4}
	}}
}};
""".format(os.path.splitext(os.path.basename(font_bmp_file))[0], unknown_char_width, char_height, len(data), ", ".join([hex(x) for x in data]))
c_code += struct_c_code

struct_h_code = """
extern const struct image char_{0}_unknown;
""".format(os.path.splitext(os.path.basename(font_bmp_file))[0]);
h_code += struct_h_code

for i in range(256):
	if i not in defined_characters:
		struct_c_font_code = """[0x{1:x}] = &char_{0}_unknown,
		""".format(os.path.splitext(os.path.basename(font_bmp_file))[0], i);
		c_font_code += struct_c_font_code

#write to files
open(sys.argv[3], "w").write(c_tmpl.format(c_code, os.path.splitext(os.path.basename(font_bmp_file))[0], c_font_code))
open(sys.argv[4], "w").write(h_tmpl.format(h_code, os.path.splitext(os.path.basename(font_bmp_file))[0]))
