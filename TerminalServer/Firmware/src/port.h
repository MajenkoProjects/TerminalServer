#ifndef _PORT_H    
#define _PORT_H

#include "definitions.h"
#include "errors.h"
#include "command.h"
#include "cbuff.h"
#include "ttype.h"

#define MAX_COMMAND         80
#define NUM_HISTORY         4

#define PORT_MAX_NAME       8

#define CONSOLE ports

enum port_setting {
    SETTING_PORT_BREAKMODE = 1,
    SETTING_PORT_ACCESS,
    SETTING_PORT_TERMINAL_TYPE,
    SETTING_PORT_LOCAL_SWITCH,
    SETTING_PORT_BACKWARD_SWITCH,
    SETTING_PORT_FORWARD_SWITCH,
};



enum port_mode {
    MODE_IDLE = 0,
    MODE_PREGREET,
    MODE_GREET,
    MODE_USERNAME,
    MODE_LOCAL,
    MODE_SESSION,
    MODE_PASSWORD,
};

enum port_type {
    PORT_NONE = 0,
    PORT_CDC,
    PORT_SERIAL,
    PORT_NET_IN,
    PORT_NET_OUT,
    PORT_TELNET_IN,
    PORT_TELNET_OUT,
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
#define LOCAL_SWITCH_ERROR -2

struct port {
    struct port *next;
    SemaphoreHandle_t mutex;
    enum port_type type;
    enum port_mode mode;
    enum port_mode previous_mode;
    struct circular_buffer read_buffer;
    struct circular_buffer write_buffer;       
    void *port_data;
    char commands[NUM_HISTORY][MAX_COMMAND];
    int cmdno;
    int cpos;
    int no;
    char name[9];
    char username[9];
    int low_water;
    int high_water;
    int waterlevel;
    bool stopped;
    int local_switch;
    int forward_switch;
    int backward_switch;
    enum break_mode breakmode;
    enum access_mode access;
    bool send_break;
    uint16_t columns;
    uint16_t lines;
    uint32_t ticks;
    char ttype[17];
    const struct ttype *tinfo;
    char keybuf[9];
    uint8_t keybuf_pos;
    struct session *active_session;
    bool priv;
    void (*fn_stop)(struct port *);
    void (*fn_start)(struct port *);
    bool (*fn_can_tx)(struct port *);
    void (*fn_close)(struct port *);
    void (*fn_show_detail)(struct port *, struct port *);
    void (*fn_flush)(struct port *);
};

extern struct port *ports;
extern const char *port_types[];
extern const char *access_names[];
extern const char *breakmode_names[];

extern void close_port(struct port *port);
extern struct port *add_port(enum port_type type, void *data);
extern void delete_port(struct port *port);

extern int port_read_byte(struct port *port);
extern int port_write_byte(struct port *port, uint8_t b);
extern int port_available(struct port *port);
extern int port_printf(struct port *port, const char *fmt, ...);
extern int port_rprintf(struct port *port, const char *fmt, ...);
extern void port_flush(struct port *port);
extern struct port *get_port_by_number(int pno);
extern struct port *get_port_by_name(const char *name);
extern int debugf(const char *fmt, ...);

extern void greet(struct port *port);
extern void port_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data);
extern const char *port_type(struct port *port);
extern void set_terminal_type(struct port *port, const char *ttype);

extern void port_set_active_session(struct port *port, struct session *session);
extern void port_set_mode(struct port *port, enum port_mode mode);
#endif 

