
#include "session.h"
#include "app.h"
#include "command.h"
#include "util.h"

struct session *sessions = NULL;

struct session *add_session(struct port *parent, struct port *target, enum session_type type) {
    
    for (struct session *scan = sessions; scan; scan = scan->next) {
        if (scan->type == SESSION_DELETED) {
            scan->parent = parent;
            scan->target = target;
            scan->type = type;
            return scan;
        }
    }
    
    struct session *new_session = (struct session *)malloc(sizeof(struct session));
    if (!new_session) return NULL;
    
    new_session->parent = parent;
    new_session->target = target;
    new_session->type = type;
    new_session->next = NULL;
    new_session->id = 1;
    
    if (sessions == NULL) {
        sessions = new_session;
        return new_session;
    }

    uint32_t max_session = 0;
    struct session *scan = sessions;
    while (scan->next) {
        if (scan->id > max_session) {
            max_session = scan->id;
        }
        scan = scan->next;
    }
    
    new_session->id = max_session + 1;
    scan->next = new_session;
    return new_session;
}

void delete_session(struct session *session) {
    session->type = SESSION_DELETED;
    session->parent = NULL;
    session->target = NULL;
}


COMMAND(connect_local) {
    
    if (argc != 1) return ERR_INCOMPLETE;
    struct port *t = get_port_by_name(argv[0]);
    if (!t) return ERR_NOTFOUND;
    
    for (struct session *scan = sessions; scan; scan = scan->next) {
        if (scan->target == t) return ERR_BUSY;
    }

    if (t->access == ACCESS_LOCAL) return ERR_BUSY;
    
    struct session *s = add_session(port, t, SESSION_DIRECT);
    port->active_session = s;
    port->mode = MODE_SESSION;
    char tmp[20];
    format_local_switch(port->local_switch, tmp);
    port_printf(port, "Local protocol emulation 1.0  - Local Switch: <%s>.\r\n", tmp);
    return ERR_OK;
}

void destroy_sessions(struct port *parent) {
    for (struct session *scan = sessions; scan; scan = scan->next) {
        if (scan->parent == parent) {
            delete_session(scan);
        }
    }
}

COMMAND(show_sessions) {
    OPT_TARGET
    if (target->active_session != NULL) {
        port_printf(port, "Port %d: %8s                 %20s    Current: %d\r\n",
            target->no, target->username, port_type(target), target->active_session->id);
    } else {
        port_printf(port, "Port %d: %8s                 %20s    Current: None\r\n",
            target->no, target->username, port_type(target));
    }
    for (struct session *scan = sessions; scan; scan = scan->next) {
        if (scan->parent == target) {
            port_printf(port, "     Session %2d  Local:%8s      Interactive   (Cr,Del)\r\n", scan->id, scan->target->name);
        }
    }
    return ERR_OK;
}

COMMAND(resume_session) {
    OPT_SESSION
    if (!session) return ERR_NOSESSION;
    if (session->parent != port) return ERR_NOSESSION;
    port->active_session = session;
    port->mode = MODE_SESSION;
    port_printf(port, "Resuming session %d\r\n", session->id);
    return ERR_OK;
}

struct session *get_session_by_number(int sid) {
    for (struct session *scan = sessions; scan; scan = scan->next) {
        if (scan->id == sid) return scan;
    }
    return NULL;    
}

COMMAND(disconnect_session) {
    OPT_SESSION
    if (!session) return ERR_NOSESSION;
    if (session->parent != port) return ERR_NOSESSION;
    port->active_session = NULL;
    delete_session(session);
    port_printf(port, "Disconnecting session %d\r\n", session->id);
    return ERR_OK;
}
