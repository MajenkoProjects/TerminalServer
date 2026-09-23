#ifndef _COMMAND_TREE_H
#define	_COMMAND_TREE_H

#include "command_help.h"

static const struct command show_port_sub[] = {
    {"CHARACTERISTICS", &show_port_characteristics, 0, NULL, NULL },
    {"STATUS", &show_port_status, 0, NULL, NULL },
    {0, 0, 0, 0, 0}
};

static const struct command show_sub[] = {
    {"PORT", &show_port_characteristics, CMD_TARGET, show_port_sub, NULL},
    {"SERVER", &show_server, 0, NULL, NULL},
    {"SESSIONS", &show_sessions, CMD_TARGET, NULL, NULL},

    {0, 0, 0, 0,0}
};

static const struct command list_sub[] = {
    {"PORTS", &list_ports, 0, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command set_port_flow_sub[] = {
    {"NONE", &port_set_flow_none, 0, NULL, NULL},
    {"RTSCTS", &port_set_flow_rts, 0, NULL, NULL},
    {"DTRDSR", &port_set_flow_dtr, 0, NULL, NULL},
    {"XONXOFF", &port_set_flow_xon, 0, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command set_port_break_sub[] = {
    {"DISABLED", &port_set_break_disabled, 0, NULL, NULL},
    {"LOCAL", &port_set_break_local, 0, NULL, NULL},
    {"REMOTE", &port_set_break_remote, 0, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command set_port_terminal_sub[] = {
    {"TYPE", &port_set_terminal_type, 0, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command set_port_local_sub[] = {
    {"SWITCH", &port_set_local_switch, 0, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command set_port_sub[] = {
    //{"ACCESS", NULL, 0, set_port_access_sub, NULL},
    {"BACKWARD", &port_set_backward_switch, 0, NULL, NULL},
    {"BREAK", NULL, 0, set_port_break_sub, NULL},
    {"FLOW", NULL, 0, set_port_flow_sub, NULL},
    {"FORWARD", &port_set_forward_switch, 0, NULL, NULL},
    {"LOCAL", &port_set_local_switch, 0, set_port_local_sub, NULL},
    //{"NAME", &port_set_name, 0, NULL, NULL},
    {"SPEED", &port_set_speed, 0, NULL, NULL},
    {"TERMINAL", NULL, 0, set_port_terminal_sub, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command set_sub[] = {
    {"NOPRIVILEGED", &set_nopriv, 0, NULL, NULL},
    {"PORT", NULL, CMD_TARGET, set_port_sub, NULL},
    {"PRIVILEGED", &set_priv, 0, NULL, NULL},
    {0, 0, 0, 0, 0}
};


static const struct command define_port_break_sub[] = {
    {"DISABLED", &port_define_break_disabled, CMD_PRIV, NULL, NULL},
    {"LOCAL", &port_define_break_local, CMD_PRIV, NULL, NULL},
    {"REMOTE", &port_define_break_remote, CMD_PRIV, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command define_port_flow_sub[] = {
    {"NONE", &port_define_flow_none, CMD_PRIV, NULL, NULL},
    {"RTSCTS", &port_define_flow_rts, CMD_PRIV, NULL, NULL},
    {"DTRDSR", &port_define_flow_dtr, CMD_PRIV, NULL, NULL},
    {"XONXOFF", &port_define_flow_xon, CMD_PRIV, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command define_port_access_sub[] = {
    {"DYNAMIC", &port_define_access_dynamic, CMD_PRIV, NULL, NULL},
    {"LOCAL", &port_define_access_local, CMD_PRIV, NULL, NULL},
    {"REMOTE", &port_define_access_remote, CMD_PRIV, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command define_port_terminal_sub[] = {
    {"TYPE", &port_define_terminal_type, CMD_PRIV, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command define_port_sub[] = {
    {"ACCESS", NULL, 0, define_port_access_sub, NULL},
    {"BACKWARD", &port_define_backward_switch, CMD_PRIV, NULL, NULL},
    {"BREAK", NULL, 0, define_port_break_sub, NULL},
    {"FLOW", NULL, 0, define_port_flow_sub, NULL},
    {"FORWARD", &port_define_forward_switch, CMD_PRIV, NULL, NULL},
    {"LOCAL", &port_define_local_switch, CMD_PRIV, NULL, NULL},
    {"NAME", &port_define_name, CMD_PRIV, NULL, NULL},
    {"SPEED", &port_define_speed, CMD_PRIV, NULL, HELP_DEFINE_PORT_SPEED},
    {"TERMINAL", NULL, 0, define_port_terminal_sub, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command define_server_mac_sub[] = {
    {"ADDRESS", &ethernet_define_mac_address, CMD_PRIV, NULL, HELP_DEFINE_SERVER_MAC_ADDRESS},
    {0, 0, 0, 0, 0}
};

static const struct command define_server_wifi_mac_sub[] = {
    {"ADDRESS", &wifi_define_mac_address, CMD_PRIV, NULL, HELP_DEFINE_SERVER_WIFI_MAC_ADDRESS},
    {0, 0, 0, 0, 0}
};

static const struct command define_server_subnet_sub[] = {
    {"MASK", &ethernet_define_subnet, CMD_PRIV, NULL, HELP_DEFINE_SERVER_SUBNET},
    {0, 0, 0, 0, 0}
};

static const struct command define_server_wifi_subnet_sub[] = {
    {"MASK", &wifi_define_subnet, CMD_PRIV, NULL, HELP_DEFINE_SERVER_WIFI_SUBNET},
    {0, 0, 0, 0, 0}
};

static const struct command define_server_secondary_sub[] = {
    {"NAMESERVER", &ethernet_define_secdns, CMD_PRIV, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command define_server_wifi_secondary_sub[] = {
    {"NAMESERVER", &wifi_define_secdns, CMD_PRIV, NULL, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command define_server_dhcp_sub[] = {
    {"DISABLED", &ethernet_define_dhcp_disabled, CMD_PRIV, NULL, HELP_DEFINE_SERVER_DHCP_DISABLED},
    {"ENABLED", &ethernet_define_dhcp_enabled, CMD_PRIV, NULL, HELP_DEFINE_SERVER_DHCP_ENABLED},
    {0, 0, 0, 0, 0}
};

static const struct command define_server_wifi_dhcp_sub[] = {
    {"DISABLED", &wifi_define_dhcp_disabled, CMD_PRIV, NULL, HELP_DEFINE_SERVER_WIFI_DHCP_DISABLED},
    {"ENABLED", &wifi_define_dhcp_enabled, CMD_PRIV, NULL, HELP_DEFINE_SERVER_WIFI_DHCP_ENABLED},
    {0, 0, 0, 0, 0}
};

static const struct command define_server_ethernet_sub[] = {
    {"DHCP", NULL, 0, define_server_dhcp_sub, NULL},
    {"GATEWAY", &ethernet_define_gateway, CMD_PRIV, NULL, HELP_DEFINE_SERVER_GATEWAY},
    {"IPADDRESS", &ethernet_define_ip, CMD_PRIV, NULL, HELP_DEFINE_SERVER_IPADDRESS},
    {"MAC", &ethernet_define_mac_address, CMD_PRIV, define_server_mac_sub, HELP_DEFINE_SERVER_MAC_ADDRESS},
    {"NAMESERVER", &ethernet_define_pridns, CMD_PRIV, NULL, HELP_DEFINE_SERVER_NAMESERVER},
    {"SECONDARY", NULL, 0, define_server_secondary_sub, NULL},
    {"SUBNET", &ethernet_define_subnet, CMD_PRIV, define_server_subnet_sub, HELP_DEFINE_SERVER_SUBNET},
    {0, 0, 0, 0, 0}
};

static const struct command define_server_wifi_sub[] = {
    {"DHCP", NULL, 0, define_server_wifi_dhcp_sub, NULL},
    {"IPADDRESS", &wifi_define_ip, CMD_PRIV, NULL, NULL},
    {"SUBNET", &wifi_define_subnet, CMD_PRIV, define_server_wifi_subnet_sub, NULL},
    {"GATEWAY", &wifi_define_gateway, CMD_PRIV, NULL, NULL},
    {"MAC", &wifi_define_mac_address, CMD_PRIV, define_server_wifi_mac_sub, NULL},
    {"NAMESERVER", &wifi_define_pridns, CMD_PRIV, NULL, NULL},
    {"PSK", &wifi_define_psk, CMD_PRIV, NULL, NULL},
    {"SECONDARY", NULL, 0, define_server_wifi_secondary_sub, NULL},
    {"SSID", &wifi_define_ssid, CMD_PRIV, NULL, NULL},
    {0, 0, 0, 0, 0}
};


static const struct command define_server_sub[] = {
    {"DHCP", NULL, 0, define_server_dhcp_sub, NULL},
    {"DOMAIN", &system_define_domain, CMD_PRIV, NULL, NULL},
    {"ETHERNET", NULL, CMD_PRIV, define_server_ethernet_sub, NULL},
    {"IPADDRESS", &ethernet_define_ip, CMD_PRIV, NULL, HELP_DEFINE_SERVER_IPADDRESS},
    {"SUBNET", &ethernet_define_subnet, CMD_PRIV, define_server_subnet_sub, HELP_DEFINE_SERVER_SUBNET},
    {"GATEWAY", &ethernet_define_gateway, CMD_PRIV, NULL, HELP_DEFINE_SERVER_GATEWAY},
    {"MAC", &ethernet_define_mac_address, CMD_PRIV, define_server_mac_sub, HELP_DEFINE_SERVER_MAC_ADDRESS},
    {"NAME", &system_define_name, CMD_PRIV, NULL, HELP_DEFINE_SERVER_NAME},
    {"NAMESERVER", &ethernet_define_pridns, CMD_PRIV, NULL, HELP_DEFINE_SERVER_NAMESERVER},
    {"SECONDARY", NULL, 0, define_server_secondary_sub, NULL},
    {"WIFI", NULL, CMD_PRIV, define_server_wifi_sub, NULL},
    {0, 0, 0, 0, 0}
};

static const struct command define_sub[] = {
    {"PORT", NULL, CMD_TARGET, define_port_sub, NULL},
    {"SERVER", NULL, 0, define_server_sub, NULL},
    {0, 0, 0, 0, 0}
};


static const struct command send_sub[] = {
    {"BREAK", &port_send_break, 0, NULL, HELP_SEND_BREAK},
    {0, 0, 0, 0, 0}
};

static const struct command connect_sub[] = {
    {"LOCAL", &connect_local, 0, NULL, HELP_CONNECT_LOCAL},
    {"TELNET", &telnet, 0, NULL, HELP_CONNECT_TELNET},
    {0, 0, 0, 0, 0}
};

static const struct command initialize_sub[] = {
    {"FACTORY", &system_factory_reset, CMD_PRIV, NULL, HELP_INITIALIZE_FACTORY},
    {0, 0, 0, 0, 0}
};

static const struct command commands[] = {
    {"CONNECT", NULL, 0, connect_sub, NULL},
    {"DEFINE", NULL, 0, define_sub, NULL},
    {"DISCONNECT", &disconnect_session, CMD_SESSION, NULL, HELP_DISCONNECT},
    {"HELP", &help, 0, NULL, HELP_HELP},
    {"INITIALIZE", NULL, 0, initialize_sub, NULL},
    {"LIST", NULL, 0, list_sub, NULL},
    {"LOGOUT", &logout, 0, NULL, HELP_LOGOUT},
    {"OPEN", &telnet, 0, NULL, HELP_CONNECT_TELNET},
    {"RESUME", &resume_session, CMD_SESSION, NULL, HELP_RESUME},
    {"SEND", NULL, 0, send_sub, NULL},
    {"SET", NULL, 0, set_sub, NULL},
    {"SHOW", NULL, 0, show_sub, NULL},
    {0, 0, 0, 0, 0}
};

#endif