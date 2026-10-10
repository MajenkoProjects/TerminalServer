#ifndef _TU58_H
#define	_TU58_H

#include "port.h"


extern void tu58_init(struct port *port);
extern void tu58_process(struct port *port);
extern void tu58_show_info(struct port *port, struct port *target);


extern bool tu58_mount(const char *filename, struct port *port, int unit);
extern bool tu58_dismount(struct port *port, int unit);
#endif

