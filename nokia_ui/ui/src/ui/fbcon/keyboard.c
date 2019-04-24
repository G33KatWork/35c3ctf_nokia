#include "keyboard.h"

#include <linux/input.h>

static const struct keysym_map {
    int  normal, shifted;
} keysym_map_en_us[KEY_CNT] = {
    [KEY_A] = { .normal = 'a', .shifted = 'A' },
    [KEY_B] = { .normal = 'b', .shifted = 'B' },
    [KEY_C] = { .normal = 'c', .shifted = 'C' },
    [KEY_D] = { .normal = 'd', .shifted = 'D' },
    [KEY_E] = { .normal = 'e', .shifted = 'E' },
    [KEY_F] = { .normal = 'f', .shifted = 'F' },
    [KEY_G] = { .normal = 'g', .shifted = 'G' },
    [KEY_H] = { .normal = 'h', .shifted = 'H' },
    [KEY_I] = { .normal = 'i', .shifted = 'I' },
    [KEY_J] = { .normal = 'j', .shifted = 'J' },
    [KEY_K] = { .normal = 'k', .shifted = 'K' },
    [KEY_L] = { .normal = 'l', .shifted = 'L' },
    [KEY_M] = { .normal = 'm', .shifted = 'M' },
    [KEY_N] = { .normal = 'n', .shifted = 'N' },
    [KEY_O] = { .normal = 'o', .shifted = 'O' },
    [KEY_P] = { .normal = 'p', .shifted = 'P' },
    [KEY_Q] = { .normal = 'q', .shifted = 'Q' },
    [KEY_R] = { .normal = 'r', .shifted = 'R' },
    [KEY_S] = { .normal = 's', .shifted = 'S' },
    [KEY_T] = { .normal = 't', .shifted = 'T' },
    [KEY_U] = { .normal = 'u', .shifted = 'U' },
    [KEY_V] = { .normal = 'v', .shifted = 'V' },
    [KEY_W] = { .normal = 'w', .shifted = 'W' },
    [KEY_X] = { .normal = 'x', .shifted = 'X' },
    [KEY_Y] = { .normal = 'y', .shifted = 'Y' },
    [KEY_Z] = { .normal = 'z', .shifted = 'Z' },

    [KEY_1] = { .normal = '1', .shifted = '!' },
    [KEY_2] = { .normal = '2', .shifted = '@' },
    [KEY_3] = { .normal = '3', .shifted = '#' },
    [KEY_4] = { .normal = '4', .shifted = '$' },
    [KEY_5] = { .normal = '5', .shifted = '%' },
    [KEY_6] = { .normal = '6', .shifted = '^' },
    [KEY_7] = { .normal = '7', .shifted = '&' },
    [KEY_8] = { .normal = '8', .shifted = '*' },
    [KEY_9] = { .normal = '9', .shifted = '(' },
    [KEY_0] = { .normal = '0', .shifted = ')' },

    [KEY_MINUS]       = { .normal = '-',  .shifted = '_'  },
    [KEY_EQUAL]       = { .normal = '=',  .shifted = '+'  },
    [KEY_TAB]         = { .normal = '\t', .shifted = '\t' },
    [KEY_LEFTBRACE]   = { .normal = '[',  .shifted = '{'  },
    [KEY_RIGHTBRACE]  = { .normal = ']',  .shifted = '}'  },
    [KEY_SEMICOLON]   = { .normal = ';',  .shifted = ':'  },
    [KEY_APOSTROPHE]  = { .normal = '"',  .shifted = '\'' },
    [KEY_BACKSLASH]   = { .normal = '\\', .shifted = '|'  },
    [KEY_COMMA]       = { .normal = ',',  .shifted = '<'  },
    [KEY_DOT]         = { .normal = '.',  .shifted = '>'  },
    [KEY_SLASH]       = { .normal = '/',  .shifted = '?'  },
    [KEY_SPACE]       = { .normal = ' ',  .shifted = ' '  },

    [KEY_BACKSPACE]   = { .normal = NOKIA_KEY_BACKSPACE,  .shifted = NOKIA_KEY_BACKSPACE },
    [KEY_UP]          = { .normal = NOKIA_KEY_UP,         .shifted = NOKIA_KEY_UP },
    [KEY_DOWN]        = { .normal = NOKIA_KEY_DOWN,       .shifted = NOKIA_KEY_DOWN },
    [KEY_LEFT]        = { .normal = NOKIA_KEY_LEFT,       .shifted = NOKIA_KEY_LEFT  },
    [KEY_RIGHT]       = { .normal = NOKIA_KEY_RIGHT,      .shifted = NOKIA_KEY_RIGHT },
    [KEY_ESC]         = { .normal = NOKIA_KEY_ESC,        .shifted = NOKIA_KEY_ESC },
    [KEY_ENTER]       = { .normal = NOKIA_KEY_ENTER,      .shifted = NOKIA_KEY_ENTER },
};

int fbcon_keyboard_key_to_character(int scancode, int shifted)
{
    if(scancode > 0 && scancode < KEY_CNT-1)
        return shifted ? keysym_map_en_us[scancode].shifted : keysym_map_en_us[scancode].normal;
    else
        return 0;
}
