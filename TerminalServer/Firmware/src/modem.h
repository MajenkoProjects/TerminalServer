#ifndef _MODEM_H
#define	_MODEM_H

#include "port.h"

enum modem_response {
    MODEM_OK = 0,
    MODEM_CONNECT,
    MODEM_RING,
    MODEM_NO_CARRIER,
    MODEM_ERROR,
    MODEM_CONNECT_1200,
    MODEM_NO_DIALTONE,
    MODEM_BUSY,
    MODEM_NO_ANSWER,
    MODEM_CONNECT_2400 = 10,
    MODEM_CONNECT_4800,
    MODEM_CONNECT_9600,
    MODEM_CONNECT_14400,
    MODEM_CONNECT_19200,
    MODEM_CONNECT_1200_75 = 22,
    MODEM_CONNECT_75_1200,
    MODEM_CONNECT_7200,
    MODEM_CONNECT_12000,
    MODEM_CONNECT_38400 = 28
};


#define MODEM_VERBOSE           0x01
#define MODEM_ECHO              0x02



extern void modem_process(struct port *port, int c);
extern bool modem_response(struct port *port, enum modem_response r);
#endif
