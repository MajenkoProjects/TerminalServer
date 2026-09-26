#ifndef _PORT_COMMANDS_H
#define _PORT_COMMANDS_H

#include "command.h"

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
extern COMMAND(port_set_terminal_type);
extern COMMAND(port_define_terminal_type);
extern COMMAND(port_set_local_switch);
extern COMMAND(port_set_forward_switch);
extern COMMAND(port_set_backward_switch);
extern COMMAND(port_define_local_switch);
extern COMMAND(port_define_forward_switch);
extern COMMAND(port_define_backward_switch);
extern COMMAND(port_set_bits);
extern COMMAND(port_define_bits);

extern COMMAND(port_set_parity_odd);
extern COMMAND(port_set_parity_even);
extern COMMAND(port_set_parity_mark);
extern COMMAND(port_set_parity_space);
extern COMMAND(port_set_parity_none);

extern COMMAND(port_define_parity_odd);
extern COMMAND(port_define_parity_even);
extern COMMAND(port_define_parity_mark);
extern COMMAND(port_define_parity_space);
extern COMMAND(port_define_parity_none);
#endif
