SOURCES = 	nokia_ui.c \
			display.c \
			images.c \
			fonts.c \
			animations.c \
			wordwrap.c \
			db.c \
			screen.c \
			ui_sock.c \
			networks.c \
			uictl.c \
			sms.c \
			ui_event.c \
			sms_receive.c

#settings.c \
#mnccctl.c \
#mncc_sock.c \

SOURCES +=  ui/$(UILIB)/ui.c \
			ui/$(UILIB)/keyboard.c

SOURCES +=	screens/startup_animation.c \
			screens/main_screen.c \
			screens/menu.c \
			screens/phone_book_browse.c \
			screens/phone_book_add.c \
			screens/phone_book_delete.c \
			screens/info.c \
			screens/progress.c \
			screens/sms_write.c \
			screens/input_dialog.c \
			screens/phone_book_browser.c \
			screens/text_editor.c \
			screens/sms_inbox.c \
			screens/sms_viewer.c \
			screens/sms_inbox_list.c \
			screens/template_create.c \
			screens/template_list.c \
			screens/template_manage.c \
			screens/template_viewer.c \
			screens/sim.c \
			screens/pin_enter.c \
			screens/error.c

SOURCES += 	menus/main_menu.c \
			menus/sms_view_options_menu.c

GENERATED =	generated/nokia_images.c \
			generated/font_large_bold.c \
			generated/font_small_bold.c \
			generated/font_small_plain.c

GENERATED_HEADERS =	generated/nokia_images.h \
					generated/font_large_bold.h \
					generated/font_small_bold.h \
					generated/font_small_plain.h

PKG_CONFIG_LIBS_sdl = sdl2
PKG_CONFIG_LIBS_fbcon = 
PKG_CONFIG_LIBS = $(PKG_CONFIG_LIBS_$(UILIB)) sqlite3 libosmocore libosmogsm

UILIB ?= sdl

SRCDIR = src
INCDIR = inc
OBJDIR = obj

#CC = gcc

CFLAGS := -O2 -Wall -std=gnu99 -I $(INCDIR) -I $(INCDIR)/generated -I $(INCDIR)/../../inc -ggdb $(foreach LIB, $(PKG_CONFIG_LIBS), $(shell pkg-config --cflags $(LIB)))
LDFLAGS := $(foreach LIB, $(PKG_CONFIG_LIBS), $(shell pkg-config --libs $(LIB)))

OBJECTS = $(SOURCES:.c=.o) $(GENERATED:.c=.o)
APP = nokia_ui

build: all
all: $(addprefix $(INCDIR)/, $(GENERATED_HEADERS)) $(addprefix $(SRCDIR)/, $(GENERATED)) $(addprefix $(SRCDIR)/, $(SOURCES)) $(OBJDIR) $(APP)
generate: $(addprefix $(INCDIR)/, $(GENERATED_HEADERS)) $(addprefix $(SRCDIR)/, $(GENERATED))

$(OBJDIR):
	mkdir -p $(OBJDIR)/generated

$(APP): $(addprefix $(OBJDIR)/, $(OBJECTS))
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(SRCDIR)/generated/%.c $(INCDIR)/generated/%.h:
	mkdir -p $(SRCDIR)/generated
	mkdir -p $(INCDIR)/generated
	tools/img_to_array.py tools/images images src/generated/nokia_images.c inc/generated/nokia_images.h
	tools/font_to_array.py tools/font_large_bold fonts src/generated/font_large_bold.c inc/generated/font_large_bold.h
	tools/font_to_array.py tools/font_small_bold fonts src/generated/font_small_bold.c inc/generated/font_small_bold.h
	tools/font_to_array.py tools/font_small_plain fonts src/generated/font_small_plain.c inc/generated/font_small_plain.h

$(OBJDIR)/%.d: $(SRCDIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MM -MG -MP -MT '$(@:%.d=%.o)' $< > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(addprefix $(OBJDIR)/, $(OBJECTS:%.o=%.d))
endif

clean:
	rm -rf $(APP)
	rm -rf $(OBJDIR)
	rm -rf $(SRCDIR)/generated/*.c
	rm -rf $(INCDIR)/generated/*.h

.PHONY: all clean
