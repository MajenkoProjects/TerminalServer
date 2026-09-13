#ifndef _PORT_H    
#define _PORT_H

#include "definitions.h"
#include "errors.h"
#include "command.h"
#include "cbuff.h"

#define MAX_COMMAND         128

#define PORT_MAX_NAME       8

#define CONSOLE ports

enum port_setting {
    SETTING_PORT_BREAKMODE = 1,
    SETTING_PORT_ACCESS,
};



enum port_mode {
    MODE_IDLE = 0,
    MODE_PREGREET,
    MODE_GREET,
    MODE_USERNAME,
    MODE_LOCAL,
    MODE_SESSION,
};

enum port_type {
    PORT_NONE = 0,
    PORT_CDC,
    PORT_SERIAL,
    PORT_NET_IN,
    PORT_NET_OUT,
    PORT_TELNET_IN,
};

enum access_mode {
    ACCESS_LOCAL = 0,
    ACCESS_REMOTE,
    ACCESS_DYNAMIC
};

enum break_mode {
    BREAK_DISABLED = 0,
    BREAK_LOCAL,
    BREAK_REMOTE
};

#define LOCAL_SWITCH_NONE -1


struct port {
    struct port *next;
    enum port_type type;
    enum port_mode mode;
    struct port *remote_port;
    struct circular_buffer read_buffer;
    struct circular_buffer write_buffer;       
    void *port_data;
    char command[MAX_COMMAND];
    int command_len;
    int no;
    char name[9];
    char username[9];
    int low_water;
    int high_water;
    int waterlevel;
    bool stopped;
    int local_switch;
    enum break_mode breakmode;
    enum access_mode access;
    bool send_break;
    uint16_t columns;
    uint16_t lines;
    uint32_t ticks;
    char ttype[17];
    struct session *active_session;
    void (*fn_stop)(struct port *);
    void (*fn_start)(struct port *);
    bool (*fn_can_tx)(struct port *);
    void (*fn_close)(struct port *);
    void (*fn_show_detail)(struct port *, struct port *);
};

extern struct port *ports;

extern void close_port(struct port *port);
extern struct port *add_port(enum port_type type, void *data);
extern void delete_port(struct port *port);

extern int port_read_byte(struct port *port);
extern int port_write_byte(struct port *port, uint8_t b);
extern int port_available(struct port *port);
extern int port_printf(struct port *port, const char *fmt, ...);
extern void port_flush(struct port *port);
extern struct port *get_port_by_number(int pno);
extern struct port *get_port_by_name(const char *name);
extern int debugf(const char *fmt, ...);

//extern error_t port_set_cmd(struct port *port, int argc, const char **argv);
extern COMMAND(show_port_characteristics);
extern COMMAND(show_port_status);
extern COMMAND(list_ports);
extern COMMAND(port_set_name);
extern COMMAND(port_set_speed);
extern COMMAND(port_define_name);
extern COMMAND(port_define_speed);

extern COMMAND(port_set_flow_none);
extern COMMAND(port_set_flow_rts);
extern COMMAND(port_set_flow_dtr);
extern COMMAND(port_set_flow_xon);

extern COMMAND(port_define_flow_none);
extern COMMAND(port_define_flow_rts);
extern COMMAND(port_define_flow_dtr);
extern COMMAND(port_define_flow_xon);

extern COMMAND(port_send_break);

extern COMMAND(port_set_access_dynamic);
extern COMMAND(port_set_access_local);
extern COMMAND(port_set_access_remote);
extern COMMAND(port_define_access_dynamic);
extern COMMAND(port_define_access_local);
extern COMMAND(port_define_access_remote);


extern COMMAND(port_set_break_disabled);
extern COMMAND(port_set_break_local);
extern COMMAND(port_set_break_remote);

extern COMMAND(port_define_break_disabled);
extern COMMAND(port_define_break_local);
extern COMMAND(port_define_break_remote);

extern void greet(struct port *port);

extern void port_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data);


const char *port_type(struct port *port);

#endif 

