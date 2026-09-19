
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

    struct session *scan = sessions;
    uint32_t max_session = scan->id;
    while (scan->next) {
        if (scan->next->id > max_session) {
            max_session = scan->next->id;
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
    port_set_active_session(port, s);
    port_set_mode(port, MODE_SESSION);
    char tmp[20];
    format_local_switch(port->local_switch, tmp, 20);

    port_printf(port, "Local protocol emulation 1.0  - Local Switch: <%s>.\r\n", tmp);
    return ERR_OK;
}

void destroy_sessions(struct port *port) {

    // Iterate all the sessions
    for (struct session *scan = sessions; scan; scan = scan->next) {
        
        // If this session is linked to the port as either parent or target
        if ((scan->parent == port) || (scan->target == port)) {
            
            // If the target can be closed, close it
            if (scan->target->fn_close) {
                scan->target->fn_close(scan->target);
            }
            
            // Delete the session.
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
            switch (scan->type) {
                case SESSION_DIRECT:
                    port_printf(port, "     Session %2d  Local:%8s      Interactive   (Cr,Del)\r\n", scan->id, scan->target->name);
                    break;
                case SESSION_CLOSING:
                    port_printf(port, "     Session %2d  Local:%8s      Closing       (Cr,Del)\r\n", scan->id, scan->target->name);
                    break;
                default:
                    break;
            }
        }
    }
    return ERR_OK;
}

COMMAND(resume_session) {
    OPT_SESSION
    if (!session) return ERR_NOSESSION;
    if (session->parent != port) return ERR_NOSESSION;
    port_set_active_session(port, session);
    port_set_mode(port, MODE_SESSION);
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
    port_printf(port, "Disconnecting session %d\r\n", session->id);
    if (session->target->fn_close) {
        session->target->fn_close(session->target);
    }
    port_set_active_session(port, NULL);
    delete_session(session);
    return ERR_OK;
}


void session_slave_close(struct port *port) {
    for (struct session *scan = sessions; scan; scan = scan->next) {
        if (scan->target == port) {
            //port_printf(CONSOLE, "Found session to kill\r\n");
            port_set_mode(scan->parent, MODE_LOCAL);
//            port_printf(scan->parent, "Connection closed\r\n");
            //port_flush(scan->target);
            //port_flush(scan->parent);
            port_set_active_session(scan->parent, NULL);
            delete_session(scan);
            return;
        }
    }
}

void session_parent_close(struct port *port) {
    for (struct session *scan = sessions; scan; scan = scan->next) {
        if (scan->parent == port) {
            //port_printf(CONSOLE, "Found session to kill\r\n");
            port_set_mode(scan->parent, MODE_LOCAL);
//            port_printf(scan->parent, "Connection closed\r\n");
            //port_flush(scan->target);
            //port_flush(scan->parent);
            port_set_active_session(scan->parent, NULL);
            delete_session(scan);
            return;
        }
    }
}