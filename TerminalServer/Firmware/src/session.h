#ifndef _SESSION_H
#define _SESSION_H

#include <stdint.h>

#include "errors.h"
#include "port.h"

enum session_type {
    SESSION_DELETED = 0,
    SESSION_DIRECT,
    SESSION_CLOSING,
};

struct session {
    struct session *next;
    struct port *parent;
    struct port *target;
    enum session_type type;
    uint32_t id;
};

extern struct session *sessions;

extern struct session *add_session(struct port *parent, struct port *target, enum session_type type);
extern COMMAND(connect_local);
extern void destroy_sessions(struct port *parent);
extern COMMAND(show_sessions);
extern COMMAND(resume_session);
extern COMMAND(disconnect_session);
extern struct session *get_session_by_number(int sid);
extern void session_slave_close(struct port *);
#endif