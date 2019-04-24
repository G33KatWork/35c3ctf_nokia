SOURCES = 	sdltest.c

PKG_CONFIG_LIBS = sdl2

SRCDIR = src
INCDIR = inc
OBJDIR = obj

CFLAGS := -Wall -std=gnu99 -I $(INCDIR) -ggdb $(foreach LIB, $(PKG_CONFIG_LIBS), $(shell pkg-config --cflags $(LIB)))
LDFLAGS := $(foreach LIB, $(PKG_CONFIG_LIBS), $(shell pkg-config --libs $(LIB)))

OBJECTS = $(SOURCES:.c=.o)
APP = sdltest

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
