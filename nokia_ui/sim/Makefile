SOURCES +=  socket/main.c \
			socket/sim.c \
			socket/socket.c \
			socket/sim_sock.c \
			socket/utils.c \
			utils.c \
			msg.c \
			apdu.c \
			apdu_chv.c \
			apdu_select.c \
			apdu_files_transparent.c \
			apdu_files_linear_cyclic.c \
			apdu_replies.c \
			apdu_misc.c \
			files.c \
			chv.c \
			comp128v1.c \
			cJSON.c

PKG_CONFIG_LIBS = 

SRCDIR = src
INCDIR = inc
OBJDIR = obj

#CC = gcc

CFLAGS := -O2 -Wall -std=gnu99 -I $(INCDIR) -I $(INCDIR)/../../inc -ggdb $(foreach LIB, $(PKG_CONFIG_LIBS), $(shell pkg-config --cflags $(LIB))) -D SIM_SOCKET
LDFLAGS := $(foreach LIB, $(PKG_CONFIG_LIBS), $(shell pkg-config --libs $(LIB)))

OBJECTS = $(SOURCES:.c=.o)
APP = sim

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
