#ifndef _COMMAND_H
#define _COMMAND_H

#define COMMAND(X) error_t X(struct port *port, void *opt, int argc, char **argv)

enum command_states {
    CMD_LOCAL,
    CMD_ASKUSERNAME,
    CMD_ASKPRIVPASS,
    CMD_SETPASS1,
    CMD_SETPASS2
};

#include "port.h"
#include "errors.h"

#define MAX_ARGS            10

#define CMD_TARGET          0x01
#define CMD_SESSION         0x02
#define CMD_PRIV            0x80

#define OPT_TARGET struct port *target = port; if (opt) target = (struct port *)opt;
#define OPT_SESSION struct session *session = port->active_session; if (opt) session = (struct session *)opt;


struct port;

struct command {
    const char *command;
    error_t (*func)(struct port *port, void *opt, int argc, char **argv);
    uint8_t flags;
    const struct command *sub_commands;
    const char *help;
};


struct command_state {
    void (*fn_execute)(struct port *);
    const char *prompt;
    const char *privprompt;
    bool echo;
};

extern int command_process(struct port *port, char c);
extern void command_execute(struct port *port);
extern const char *prompt(struct port *);

extern COMMAND(show_server);
extern COMMAND(logout);
extern COMMAND(set_priv);
extern COMMAND(set_nopriv);
extern COMMAND(help);
extern COMMAND(show_command_tree);
#endif