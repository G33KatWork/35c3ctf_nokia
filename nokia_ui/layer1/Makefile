SOURCES +=  virt_prim_rach.c \
			virt_prim_data.c \
			virt_prim_fbsb.c \
			virt_prim_traffic.c \
			virt_l1_sched_simple.c \
			virt_l1_model.c \
			virt_prim_pm.c \
			l1ctl_sock.c \
			l1ctl_sap.c \
			gsmtapl1_if.c \
			virtphy.c \
			logging.c \
			shared/osmo_mcast_sock.c \
			shared/virtual_um.c

PKG_CONFIG_LIBS = libosmocore libosmogsm

SRCDIR = src
INCDIR = inc
OBJDIR = obj

#CC = gcc

CFLAGS := -O2 -Wall -std=gnu99 -I $(INCDIR) -I $(INCDIR)/../../inc -ggdb $(foreach LIB, $(PKG_CONFIG_LIBS), $(shell pkg-config --cflags $(LIB)))
LDFLAGS := $(foreach LIB, $(PKG_CONFIG_LIBS), $(shell pkg-config --libs $(LIB)))

OBJECTS = $(SOURCES:.c=.o)
APP = layer1

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
