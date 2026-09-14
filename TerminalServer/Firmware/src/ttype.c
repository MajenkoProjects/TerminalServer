#include <stdio.h>
#include "ttype.h"

const struct ttype dumb = {
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


const struct ttype vt100 = {
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

const struct ttype vt102 = {
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
    "\e[1@",
    "\e[1P",
};