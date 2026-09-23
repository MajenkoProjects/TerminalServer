#ifndef _COMMAND_HELP_H
#define	_COMMAND_HELP_H


#define HELP_RESUME \
"RESUME [<session>]\r\n" \
"\n" \
"Resume a session from your list of active sessions. Takes an optional session\r\n" \
"number to resume a session that is not your currently active session.\r\n"

#define HELP_LOGOUT \
"LOGOUT\r\n" \
"\n" \
"Close all open sessions and return to the Username> prompt. If this port is a\r\n" \
"transient network port the connection will be closed and you will be returned\r\n" \
"to your local system.\r\n"

#define HELP_HELP \
"HELP\r\n" \
"\n" \
"Find help on using a specific command. Type 'HELP' followed by the command you\r\n" \
"are interested. If that command is part of a longer command sequence the\r\n" \
"possible next words in the command sequence are listed. Otherwise a section of\r\n" \
"text describing what the command does and how to use it is printed. If you ask\r\n" \
"for help on a command that does not exist a short message telling you the\r\n" \
"command does not exist is printed.\r\n"
        
#define HELP_DISCONNECT \
"DISCONNECT\r\n" \
"\n" \
"Disconnect a session. A session is any connection between your current terminal\r\n" \
"and a local port or remote system. Takes an optional session number to\r\n" \
"disconnect (see SHOW SESSIONS). If omitted it disconnects your currently active\r\n" \
"session.\r\n"

#define HELP_INITIALIZE_FACTORY \
"INITIALIZE FACTORY\r\n" \
"\n" \
"Resets the whole system back to factory settings - all ports, passwords, session\r\n" \
"data. Can also be performed by holding the RESET button on the rear of the unit\r\n" \
"for 30 seconds.\r\n"

#define HELP_CONNECT_TELNET \
"CONNECT TELNET <host>[:<port>] (alias: OPEN)\r\n" \
"\n" \
"Connect to a remote host through the TELNET protocol. Optionally takes a colon\r\n" \
"separated port number to connect to.\r\n"

#define HELP_CONNECT_LOCAL \
"CONNECT LOCAL <port>\r\n" \
"\n" \
"Connect to a local port, either by name or by number. The port you are trying\r\n" \
"to connect to must be in either REMOTE or DYNAMIC access mode (see SET PORT\r\n" \
"ACCESS) for this command to work. Can be used on either Serial or USB ports.\r\n"

#define HELP_DEFINE_SERVER_NAMESERVER \
"DEFINE SERVER [ETHERNET] NAMESERVER <ip>\r\n" \
"\n" \
"Set the primary name server for the Ethernet connection for use whan not using\r\n" \
"DHCP for obtaining network settings automatically. See also DEFINE SERVER\r\n" \
"SECONDARY NAMESERVER. Takes effect on next reboot.\r\n"


#define HELP_DEFINE_SERVER_NAME \
"DEFINE SERVER NAME <name>\r\n" \
"\n" \
"Sets the node name for this terminal server. Used in mDNS advertisements and\r\n" \
"for general identification. Takes ffect on next reboot.\r\n"

#define HELP_DEFINE_SERVER_MAC_ADDRESS \
"DEFINE SERVER [ETHERNET] MAC [ADDRESS] <mac>\r\n" \
"\n" \
"Set the Media Access Conrtrol (MAC) address of the Ethernet interface. A MAC\r\n" \
"address is in the form of 6 hexadecimal (0-9,A-F) pairs separated by either\r\n" \
"colons or hyphens. Takes effect on next reboot.\r\n"

#define HELP_DEFINE_SERVER_GATEWAY \
"DEFINE SERVER [ETHERNET] GATEWAY <ip>\r\n" \
"\n" \
"Set the default gateway for the Ethernet connection when not using DHCP for\r\n" \
"automatic configuration. Takes effect on next reboot.\r\n"

#define HELP_DEFINE_SERVER_SUBNET \
"DEFINE SERVER [ETHERNET] SUBNET [MASK] <mask>\r\n" \
"\n" \
"Set the subnet network mask, in dotted decimal notation, for the Ethernet\r\n" \
"interface. Takes effect on next reboot.\r\n"

#define HELP_DEFINE_SERVER_IPADDRESS \
"DEFINE SERVER [ETHERNET] IPADDRESS <ip>\r\n" \
"\n" \
"Set the static IP address of the Ethernet interface when not using DHCP for\r\n" \
"automatic configuration. Takes effect on next reboot.\r\n"

#define HELP_DEFINE_PORT_SPEED \
"DEFINE PORT [<port>] SPEED <baud>\r\n" \
"\n" \
"Set the baud rate of a Serial port. Takes an optional port number, otherwise\r\n" \
"it operates on the current port. Any symmetric baud rate up to 1Mbaud is\r\n" \
"supported. Note that asymmetric baud rates, like 1200/75 are not supported.\r\n" \
"Takes effect on next reboot.\r\n"

#define HELP_DEFINE_SERVER_DHCP_ENABLED \
"DEFINE SERVER [ETHERNET] DHCP ENABLED\r\n" \
"\n" \
"Enable DHCP client operation for the Ethernet port. This will obtain IP address\r\n" \
"information from a DHCP server for the Ethernet port, including IP address,\r\n" \
"default gateway, and name server addresses. This overrides any static settings\r\n" \
"and takes effect on the next reboot.\r\n"

#define HELP_DEFINE_SERVER_DHCP_DISABLED \
"DEFINE SERVER [ETHERNET] DHCP DISABLED\r\n" \
"\n" \
"Disable DHCP client operation for the Ethernet port. The statically defined\r\n" \
"IP information will be used instead of automatically obtained information. Takes\r\n" \
"effect on the next reboot.\r\n"

#define HELP_DEFINE_SERVER_WIFI_DHCP_ENABLED \
"DEFINE SERVER WIFI DHCP ENABLED\r\n" \
"\n" \
"Enable DHCP client operation for the WiFi port. This will obtain IP address\r\n" \
"information from a DHCP server for the WiFi port, including IP address,\r\n" \
"default gateway, and name server addresses. This overrides any static settings\r\n" \
"and takes effect on the next reboot.\r\n"

#define HELP_DEFINE_SERVER_WIFI_DHCP_DISABLED \
"DEFINE SERVER WIFI DHCP DISABLED\r\n" \
"\n" \
"Disable DHCP client operation for the WiFi port. The statically defined\r\n" \
"IP information will be used instead of automatically obtained information. Takes\r\n" \
"effect on the next reboot.\r\n"

#define HELP_DEFINE_SERVER_WIFI_MAC_ADDRESS \
"DEFINE SERVER WIFI MAC [ADDRESS] <mac>\r\n" \
"\n" \
"Set the Media Access Conrtrol (MAC) address of the WiFi interface. A MAC\r\n" \
"address is in the form of 6 hexadecimal (0-9,A-F) pairs separated by either\r\n" \
"colons or hyphens. Takes effect on next reboot.\r\n"

#define HELP_DEFINE_SERVER_WIFI_SUBNET \
"DEFINE SERVER WiFi SUBNET [MASK] <mask>\r\n" \
"\n" \
"Set the subnet network mask, in dotted decimal notation, for the WiFi\r\n" \
"interface. Takes effect on next reboot.\r\n"


#define HELP_SEND_BREAK \
"SEND BREAK\r\n" \
"\n" \
"Sends a BREAK signal to your currently connected session. Only has any effect\r\n" \
"on Serial ports.\r\n"


#endif

        
