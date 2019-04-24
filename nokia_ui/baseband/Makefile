SOURCES +=  common/utils.c \
			common/l1ctl.c \
			common/l1l2_interface.c \
			common/sim.c \
			common/logging.c \
			common/l1ctl_lapdm_glue.c \
			common/networks.c \
			common/sysinfo.c \
			common/sim_socket_interface.c \
			mobile/gsm411_sms.c \
			mobile/gsm48_mm.c \
			mobile/gsm48_rr.c \
			mobile/gsm48_ss.c \
			mobile/gsm322.c \
			mobile/subscriber.c \
			mobile/transaction.c \
			mobile/primitives.c \
			mobile/support.c \
			mobile/settings.c \
			mobile/ui_sock.c \
			mobile/app_mobile.c \
			mobile/main.c

TZ_SIM_ENABLE ?= no

TZ_SIM_CFLAGS_yes = -I$(TEEC_EXPORT)/include -I../sim/inc/tz -DSIM_TRUSTZONE
TZ_SIM_CFLAGS_no = 
TZ_SIM_CFLAGS = $(TZ_SIM_CFLAGS_$(TZ_SIM_ENABLE))

TZ_SIM_LDFLAGS_yes = -lteec -L$(TEEC_EXPORT)/lib
TZ_SIM_LDFLAGS_no = 
TZ_SIM_LDFLAGS = $(TZ_SIM_LDFLAGS_$(TZ_SIM_ENABLE))

TZ_SIM_SOURCES_yes = common/sim_trustzone_interface.c
TZ_SIM_SOURCES_no = 
TZ_SIM_SOURCES := $(TZ_SIM_SOURCES_$(TZ_SIM_ENABLE))

PKG_CONFIG_LIBS = libosmocore libosmogsm

SRCDIR = src
INCDIR = inc
OBJDIR = obj

SOURCES += $(TZ_SIM_SOURCES)

#CC = gcc

CFLAGS := -O2 -Wall -std=gnu99 -I $(INCDIR) -I $(INCDIR)/../../inc -ggdb $(foreach LIB, $(PKG_CONFIG_LIBS), $(shell pkg-config --cflags $(LIB))) $(TZ_SIM_CFLAGS)
LDFLAGS := $(foreach LIB, $(PKG_CONFIG_LIBS), $(shell pkg-config --libs $(LIB))) $(TZ_SIM_LDFLAGS)

OBJECTS = $(SOURCES:.c=.o)
APP = baseband

build: all
all: $(addprefix $(SRCDIR)/, $(SOURCES)) $(OBJDIR) $(APP)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(APP): $(addprefix $(OBJDIR)/, $(OBJECTS))
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJDIR)/%.d: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MM -MG -MP -MT '$(@:%.d=%.o)' $< > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(addprefix $(OBJDIR)/, $(OBJECTS:%.o=%.d))
endif

clean:
	rm -rf $(APP)
	rm -rf $(OBJDIR)

.PHONY: all clean
