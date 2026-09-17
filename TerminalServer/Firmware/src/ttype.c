#include <stdio.h>
#include "ttype.h"


const char *key_names[] = {
    "Return",
    "Backspace",
    "Delete",
    "Up",
    "Down",
    "Left",
    "Right",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "PF1",
    "PF2",
    "PF3",
    "PF4",
    "Do",
    "Help",
    "Enter"
};

const struct ttype ttype_dumb = {
    80, 24,
    {
        "\r", // Return
        "\x08", // Backspace
        "\x7F", // Del
        NULL, // Up
        NULL, // Down
        NULL, // Left
        NULL, // Right
        NULL, // F1
        NULL, // F2
        NULL, // F3
        NULL, // F4
        NULL, // F5
        NULL, // F6
        NULL, // F7
        NULL, // F8
        NULL, // F9
        NULL, // F10
        NULL, // F11
        NULL, // F12
        NULL, // F13
        NULL, // F14
        NULL, // F15
        NULL, // F16
        NULL, // F17
        NULL, // F18
        NULL, // F19
        NULL, // F20
        NULL, // F21
        NULL, // F22
        NULL, // F23
        NULL, // F24
        NULL, // PF1
        NULL, // PF2
        NULL, // PF3
        NULL, // PF4
        NULL, // DO
        NULL, // HELP
        NULL, // Enter
    },
    NULL,
    "\x08",
    NULL,
    NULL,
    NULL,
};


const struct ttype ttype_vt100 = {
    80, 24,
    {
        "\r", // Return
        "\x08", // Backspace
        "\x7F", // Del
        "\e[A", // Up
        "\e[B", // Down
        "\e[D", // Left
        "\e[C", // Right
        "\eOP", // F1
        "\eOQ", // F2
        "\eOR", // F3
        "\eOS", // F4
        "\eOt", // F5
        "\eOu", // F6
        "\eOv", // F7
        "\eOl", // F8
        "\eOw", // F9
        "\eOx", // F10
        NULL, // F11
        NULL, // F12
        NULL, // F13
        NULL, // F14
        NULL, // F15
        NULL, // F16
        NULL, // F17
        NULL, // F18
        NULL, // F19
        NULL, // F20
        NULL, // F21
        NULL, // F22
        NULL, // F23
        NULL, // F24
        "\eP", // PF1
        "\eQ", // PF2
        "\eR", // PF3
        "\eS", // PF4
        NULL, // DO
        NULL, // HELP
        "\eOM", // Enter
    },
    "\e[0K",
    "\x08",
    "\e[1C",
    NULL,
    NULL,
};

const struct ttype ttype_vt102 = {
    80, 24,
    {
        "\r", // Return
        "\x08", // Backspace
        "\e[3~", // Del
        "\e[A", // Up
        "\e[B", // Down
        "\e[D", // Left
        "\e[C", // Right
        "\e[11~", // F1
        "\e[12~", // F2
        "\e[13~", // F3
        "\e[14~", // F4
        "\e[16~", // F5
        "\e[17~", // F6
        "\e[18~", // F7
        "\e[19~", // F8
        "\e[20~", // F9
        "\e[21~", // F10
        "\e[23~", // F11
        "\e[24~", // F12
        NULL, // F13
        NULL, // F14
        NULL, // F15
        NULL, // F16
        NULL, // F17
        NULL, // F18
        NULL, // F19
        NULL, // F20
        NULL, // F21
        NULL, // F22
        NULL, // F23
        NULL, // F24
        "\eOP", // PF1
        "\eOQ", // PF2
        "\eOR", // PF3
        "\eOS", // PF4
        NULL, // DO
        NULL, // HELP
        "\eOM", // Enter
    },
    "\e[0K",
    "\x08",
    "\e[1C",
    "\e[1@",
    "\e[1P",
};

const struct ttype ttype_ansi = {
    80, 24,
    {
        "\r", // Return
        "\x7f", // Backspace
        "\e[3~", // Del
        "\e[A", // Up
        "\e[B", // Down
        "\e[D", // Left
        "\e[C", // Right
        "\eOP", // F1
        "\eOQ", // F2
        "\eOR", // F3
        "\eOS", // F4
        "\e[15~", // F5
        "\e[17~", // F6
        "\e[18~", // F7
        "\e[19~", // F8
        "\e[20~", // F9
        "\e[21~", // F10
        "\e[23~", // F11
        "\e[24~", // F12
        "\eO2P", // F13
        "\eO2Q", // F14
        "\eO2R", // F15
        "\eO2S", // F16
        "\e[15;2~", // F17
        "\e[17;2~", // F18
        "\e[18;2~", // F19
        "\e[19;2~", // F20
        "\e[20;2~", // F21
        "\e[21;2~", // F22
        "\e[23;2~", // F23
        "\e[24;2~", // F24
        NULL, // PF1
        NULL, // PF2
        NULL, // PF3
        NULL, // PF4
        NULL, // DO
        NULL, // HELP
        NULL, // Enter
    },
    "\e[0K",
    "\x08",
    "\e[1C",
    "\e[1@",
    "\e[1P",
};


const struct ttype_map ttype_map[] = {
    {"VT100", &ttype_vt100, false},
    {"VT102", &ttype_vt102, false},
    {"VT2", &ttype_ansi, true},
    {"VT3", &ttype_ansi, true},
    {"VT4", &ttype_ansi, true},
    {"XTERM", &ttype_ansi, true},
    {"ALACRITTY", &ttype_ansi, false},
    {0, 0, 0}
};