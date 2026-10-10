#ifndef _TAPES_H
#define	_TAPES_H

#include "tu58.h"
#include "command.h"

struct tape_functions {
    bool (*mount)(const char *filename, struct port *port, int unit);
    bool (*dismount)(struct port *port, int unit);
};

extern COMMAND(tapes_mount);

#endif
