#ifndef _ERRORS_H
#define _ERRORS_H

typedef enum error {
    ERR_OK = 0,
    ERR_UNKNOWN,
    ERR_INCOMPLETE,
    ERR_NOTFOUND,
    ERR_BADPORT,
    ERR_AMBIGUOUS,
    ERR_BUSY,
    ERR_NOSESSION,
    ERR_SPACES,
    ERR_TOOLONG,
    ERR_INVALID,
    ERR_PRIV,
    ERR_LAST_ERROR
} error_t;

extern const char *error_strings[ERR_LAST_ERROR];

#endif