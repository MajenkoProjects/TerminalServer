#include "errors.h"

const char *error_strings[ERR_LAST_ERROR] = {
    "OK",
    "Unknown command",
    "Incomplete command",
    "Not found",
    "Bad or incompatible port",
    "Ambiguous command",
    "Port busy",
    "No active session",
    "Spaces not allowed",
    "Option too long",
    "Invalid argument",
    "Command is privileged"
};