global-incdirs-y += inc inc/tz
cflags-y += -Wno-unused-parameter -Wno-unused-function -Wno-pedantic -Wno-format-truncation -Wno-cast-align -Wno-unused-variable -Wno-sign-compare -Wno-strict-aliasing -Wno-switch-default -Wno-float-equal
srcs-y += src/tz/main.c \
	src/tz/sim.c \
	src/tz/utils.c \
	src/utils.c \
	src/msg.c \
	src/apdu.c \
	src/apdu_chv.c \
	src/apdu_select.c \
	src/apdu_files_transparent.c \
	src/apdu_files_linear_cyclic.c \
	src/apdu_replies.c \
	src/apdu_misc.c \
	src/files.c \
	src/chv.c \
	src/comp128v1.c \
	src/cJSON.c
