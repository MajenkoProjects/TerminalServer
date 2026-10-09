#ifndef _PORT_H    
#define _PORT_H

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
//#include "definitions.h"
#include "errors.h"
#include "command.h"
#include "stream_buffer.h"
#include "ttype.h"

#define DBG(...) port_printf(CONSOLE, __VA_ARGS__); 
//CONSOLE->fn_flush(CONSOLE);

#define CIRCULAR_BUFFER_SIZE 32
#define MAX_COMMAND         40
#define NUM_HISTORY         4
#define PORT_MAX_NAME       8
#define CONSOLE ports

enum parity {
    PARITY_NONE,
    PARITY_ODD,
    PARITY_EVEN,
    PARITY_MARK,
    PARITY_SPACE
};

enum flow {
    FLOW_NONE,
    FLOW_RTSCTS,
    FLOW_DTRDSR,
    FLOW_XONXOFF
};

enum port_setting {
    SETTING_PORT_BREAKMODE = 1,
    SETTING_PORT_ACCESS,
    SETTING_PORT_TERMINAL_TYPE,
    SETTING_PORT_LOCAL_SWITCH,
    SETTING_PORT_BACKWARD_SWITCH,
    SETTING_PORT_FORWARD_SWITCH,
    SETTING_PORT_NAME,
};

enum port_mode {
    MODE_IDLE = 0,
    MODE_PREGREET,
    MODE_GREET,
    MODE_LOCAL,
    MODE_SESSION,
    MODE_MODEM,
    MODE_TU58
};

enum port_type {
    PORT_NONE = 0,
    PORT_CDC,
    PORT_SERIAL,
    PORT_NET_IN,
    PORT_NET_OUT,
    PORT_TELNET_IN,
    PORT_TELNET_OUT,
    PORT_TCP_IN,
};

enum access_mode {
    ACCESS_LOCAL = 0,
    ACCESS_REMOTE,
    ACCESS_DYNAMIC,
    ACCESS_MODEM,
    ACCESS_TU58
};

enum break_mode {
    BREAK_DISABLED = 0,
    BREAK_LOCAL,
    BREAK_REMOTE
};

#define LOCAL_SWITCH_NONE -1
#define LOCAL_SWITCH_ERROR -2

struct port;

struct port_setting_functions {
    error_t (*speed)(struct port *port, uint32_t speed);
    error_t (*parity)(struct port *port, enum parity speed);
    error_t (*bits)(struct port *port, uint8_t bits);
    error_t (*stop)(struct port *port, uint8_t stop);
    error_t (*flow)(struct port *port, enum flow flow);
};

struct port {
    struct port *next;
    enum port_type type;
    enum port_mode mode;
    enum port_mode previous_mode;
    StreamBufferHandle_t read_buffer;
    StreamBufferHandle_t write_buffer;
//    struct circular_buffer read_buffer;
//    struct circular_buffer write_buffer;       
    void *port_data;
    char commands[NUM_HISTORY][MAX_COMMAND];
    uint8_t misc[64];
    int cmdno;
    enum command_states cstate;
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
    struct port_setting_functions set;
    struct port_setting_functions define;
    void (*fn_stop)(struct port *);
    void (*fn_start)(struct port *);
    bool (*fn_can_tx)(struct port *);
    void (*fn_close)(struct port *);
    void (*fn_show_detail)(struct port *, struct port *);
    void (*fn_flush)(struct port *);
    void (*fn_yield)(struct port *);
    error_t (*fn_status)(struct port *, struct port *);
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

