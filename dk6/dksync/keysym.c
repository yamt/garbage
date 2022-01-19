#include <stddef.h>

#include "keysym.h"

const char *const symstr[] = {
        [0x03] = "MACRO",

        [0x04] = "A",
        [0x05] = "B",
        [0x06] = "C",
        [0x07] = "D",
        [0x08] = "E",
        [0x09] = "F",
        [0x0a] = "G",
        [0x0b] = "H",
        [0x0c] = "I",
        [0x0d] = "J",
        [0x0e] = "K",
        [0x0f] = "L",
        [0x10] = "M",
        [0x11] = "N",
        [0x12] = "O",
        [0x13] = "P",
        [0x14] = "Q",
        [0x15] = "R",
        [0x16] = "S",
        [0x17] = "T",
        [0x18] = "U",
        [0x19] = "V",
        [0x1a] = "W",
        [0x1b] = "X",
        [0x1c] = "Y",
        [0x1d] = "Z",

        [0x1e] = "1",
        [0x1f] = "2",
        [0x20] = "3",
        [0x21] = "4",
        [0x22] = "5",
        [0x23] = "6",
        [0x24] = "7",
        [0x25] = "8",
        [0x26] = "9",
        [0x27] = "0",

        [0x28] = "ENTER",
        [0x29] = "ESCAPE",
        [0x2a] = "BACKSPACE",
        [0x2b] = "TAB",
        [0x2c] = "SPACE",

        [0x2d] = "-",
        [0x2e] = "=",
        [0x2f] = "[",
        [0x30] = "]",
        [0x31] = "\\",
        [0x32] = "#",
        [0x33] = ";",
        [0x34] = "'",
        [0x35] = "`",
        [0x36] = ",",
        [0x37] = ".",
        [0x38] = "/",

        [0x39] = "CAPS LOCK",

        [0x3a] = "F1",
        [0x3b] = "F2",
        [0x3c] = "F3",
        [0x3d] = "F4",
        [0x3e] = "F5",
        [0x3f] = "F6",
        [0x40] = "F7",
        [0x41] = "F8",
        [0x42] = "F9",
        [0x43] = "F10",
        [0x44] = "F11",
        [0x45] = "F12",

        [0x46] = "PRINT SCREEN",
        [0x47] = "SCROLL LOCK",
        [0x48] = "PAUSE",
        [0x49] = "INSERT",
        [0x4a] = "HOME",
        [0x4b] = "PAGE UP",
        [0x4c] = "DELETE",
        [0x4d] = "END",
        [0x4e] = "PAGE DOWN",
        [0x4f] = "RIGHT ARROW",
        [0x50] = "LEFT ARROW",
        [0x51] = "DOWN ARROW",
        [0x52] = "UP ARROW",

        [0x53] = "KEYPAD NUMLOCK",
        [0x54] = "KEYPAD /",
        [0x55] = "KEYPAD *",
        [0x56] = "KEYPAD -",
        [0x57] = "KEYPAD +",
        [0x58] = "KEYPAD ENTER",
        [0x59] = "KEYPAD 1",
        [0x5a] = "KEYPAD 2",
        [0x5b] = "KEYPAD 3",
        [0x5c] = "KEYPAD 4",
        [0x5d] = "KEYPAD 5",
        [0x5e] = "KEYPAD 6",
        [0x5f] = "KEYPAD 7",
        [0x60] = "KEYPAD 8",
        [0x61] = "KEYPAD 9",
        [0x62] = "KEYPAD 0",
        [0x63] = "KEYPAD \\",

        [0x65] = "APPLICATION",
        [0x66] = "POWER",

        [0x67] = "KEYPAD =",

        [0x68] = "F13",
        [0x69] = "F14",
        [0x6a] = "F15",
        [0x6b] = "F16",
        [0x6c] = "F17",
        [0x6d] = "F18",
        [0x6e] = "F19",

        [0xd0] = "LAYER 1",
        [0xd1] = "LAYER 2",
        [0xd2] = "LAYER 3",
        [0xd3] = "LAYER 4",

        [0xd4] = "LAYER TOGGLE 1",
        [0xd5] = "LAYER TOGGLE 2",
        [0xd6] = "LAYER TOGGLE 3",
        [0xd7] = "LAYER TOGGLE 4",

        [0xdc] = "LAYER KEY 1",
        [0xdd] = "LAYER KEY 2",
        [0xde] = "LAYER KEY 3",
        [0xdf] = "LAYER KEY 4",

        [0xe0] = "LEFT CONTROL",
        [0xe1] = "LEFT SHIFT",
        [0xe2] = "LEFT OPTION",
        [0xe3] = "LEFT COMMAND",

        [0xe4] = "RIGHT CONTROL",
        [0xe5] = "RIGHT SHIFT",
        [0xe6] = "RIGHT OPTION",
        [0xe7] = "RIGHT COMMAND",

        [0xff] = "TRANSPARENT",
        [0xfe] = "DISABLED",
};

const char *
keysymstr(uint8_t v)
{
        const char *str = symstr[v];
        if (str == NULL) {
                return "unknown";
        }
        return str;
}
