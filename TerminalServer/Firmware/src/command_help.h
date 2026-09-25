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

#define HELP_DEFINE_SERVER_WIFI_NAMESERVER \
"DEFINE SERVER WIFI NAMESERVER <ip>\r\n" \
"\n" \
"Set the primary name server for the WiFi connection for use whan not using\r\n" \
"DHCP for obtaining network settings automatically. See also DEFINE SERVER\r\n" \
"WIFI SECONDARY NAMESERVER. Takes effect on next reboot.\r\n"

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

#define HELP_DEFINE_SERVER_WIFI_GATEWAY \
"DEFINE SERVER WIFI GATEWAY <ip>\r\n" \
"\n" \
"Set the default gateway for the WiFi connection when not using DHCP for\r\n" \
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

#define HELP_DEFINE_SERVER_WIFI_IPADDRESS \
"DEFINE SERVER WIFI IPADDRESS <ip>\r\n" \
"\n" \
"Set the static IP address of the WiFi interface when not using DHCP for\r\n" \
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

#define HELP_SHOW_COMMAND_TREE \
"SHOW COMMAND TREE\r\n" \
"\n" \
"Show a tree view of every command available on the system.\r\n"

#define HELP_DEFINE_PORT_ACCESS_DYNAMIC \
"DEFINE PORT [<port>] ACCESS DYNAMIC\r\n" \
"\n" \
"Sets a port to operate in Dynamic access mode. In Dynamic access mode a port\r\n" \
"is both Remote and Local at the same time. Under normal operation a Local>\r\n" \
"prompt is presented on the port for interaction by a user, but the option to\r\n" \
"connect to the port from another port or session remains available. Takes\r\n" \
"after the next reboot.\r\n"

#define HELP_DEFINE_PORT_ACCESS_LOCAL \
"DEFINE PORT [<port>] ACCESS LOCAL\r\n" \
"\n" \
"Sets a port to operate solely in Local mode. In Local mode a Local> prompt is\r\n" \
"presented on the port for interaction by a user. It is not possible to connect\r\n" \
"to the port from another port or session. Takes effect after the next reboot.\r\n"

#define HELP_DEFINE_PORT_ACCESS_REMOTE \
"DEFINE PORT [<port>] ACCESS REMOTE\r\n" \
"\n" \
"Sets a port to operate solely in Remote mode. In Remote mode the port is\r\n" \
"inactive until another port or session connects to that port at which point\r\n" \
"the port becomes active and communicates with the connected device. Takes\r\n" \
"effect after the next reboot.\r\n"



#define HELP_DEFINE_PORT_BACKWARD \
"DEFINE PORT [<port>] BACKWARD <key>\r\n" \
"\n" \
"Sets the keypress to switch to the previous session when connected to a remote\r\n" \
"system. For control keys prefix the desired character with ^. Symbolic names\r\n" \
"for many standard special keys (F1, PF3, etc) are available. Takes effect after\r\n" \
"the next reboot.\r\n"

#define HELP_DEFINE_PORT_FORWARD \
"DEFINE PORT [<port>] FORWARD <key>\r\n" \
"\n" \
"Sets the keypress to switch to the next session when connected to a remote\r\n" \
"system. For control keys prefix the desired character with ^. Symbolic names\r\n" \
"for many standard special keys (F1, PF3, etc) are available. Takes effect after\r\n" \
"the next reboot.\r\n"

#define HELP_DEFINE_PORT_LOCAL \
"DEFINE PORT [<port>] LOCAL <key>\r\n" \
"\n" \
"Sets the keypress to switch to the Local> prompt when connected to a remote\r\n" \
"system. For control keys prefix the desired character with ^. Symbolic names\r\n" \
"for many standard special keys (F1, PF3, etc) are available. Takes effect after\r\n" \
"the next reboot.\r\n"

#define HELP_DEFINE_PORT_BREAK_LOCAL \
"DEFINE PORT [<port>] BREAK LOCAL\r\n" \
"\n" \
"Sets the Break mode to Local on a selected, or the current, port. In Local\r\n" \
"Break mode any Break signal received on the selected port will cause that port\r\n" \
"to enter the Local> prompt for user interaction. Takes effect after the next\r\n" \
"reboot.\r\n"

#define HELP_DEFINE_PORT_BREAK_REMOTE \
"DEFINE PORT [<port>] BREAK REMOTE\r\n" \
"\n" \
"Sets the Break mode to Remote on a selected, or the current, port. In Remote\r\n" \
"Break mode any Break signal received on the selected port will be forwarded to\r\n" \
"the system currently being communicated with, if any. Takes effect after the\r\n" \
"next reboot.\r\n"

#define HELP_DEFINE_PORT_BREAK_DISABLED \
"DEFINE PORT [<port>] BREAK DISABLED\r\n" \
"\n" \
"Sets the Break mode to Disabled on a selected, or the current, port. In Disabled\r\n" \
"Break mode any Break signal received on the selected port will be ignored.\r\n" \
"Takes effect after the next reboot.\r\n"

#define HELP_DEFINE_PORT_FLOW_NONE \
"DEFINE PORT [<port>] FLOW NONE\r\n" \
"\n" \
"Disables flow control on the specified (or current if ommitted) port. Only\r\n" \
"applicable to Serial ports. Takes effect after next reboot.\r\n"

#define HELP_DEFINE_PORT_FLOW_XONXOFF \
"DEFINE PORT [<port>] FLOW XONXOFF\r\n" \
"\n" \
"Set the flow control on the specified (or current if ommitted) port to XON/XOFF.\r\n" \
"Only applicable to Serial ports. Takes effect after the next reboot.\r\n"

#define HELP_DEFINE_PORT_FLOW_RTSCTS \
"DEFINE PORT [<port>] FLOW RTSCTS\r\n" \
"\n" \
"Set the flow control on the specified (or current if ommitted) port to RTS/CTS.\r\n" \
"Only applicable to Serial ports. Takes effect after the next reboot.\r\n"

#define HELP_DEFINE_PORT_FLOW_DTRDSR \
"DEFINE PORT [<port>] FLOW HELP_DEFINE_PORT_FLOW_DTRDSR\r\n" \
"\n" \
"Set the flow control on the specified (or current if ommitted) port to DTR/DSR.\r\n" \
"Only applicable to Serial ports. Takes effect after the next reboot.\r\n"

#define HELP_DEFINE_PORT_NAME \
"DEFINE PORT [<port>] NAME <name>\r\n" \
"\n" \
"Set the name of a port. Used for easy identification of the device connected\r\n" \
"to the port, and with CONNECT LOCAL. Takes effect after the next reboot.\r\n"

#define HELP_DEFINE_SERVER_DOMAIN \
"DEFINE SERVER DOMAIN [NAME] <name>\r\n" \
"\n" \
"Sets the server's default DNS domain name for use in hostname lookups. Takes\r\n" \
"effect after the next reboot.\r\n"

#define HELP_DEFINE_PORT_TERMINAL_TYPE \
"DEFINE PORT [<port>] TERMINAL TYPE <type>\r\n" \
"\n" \
"Sets the terminal type for a port. Used for command line control and telnet\r\n" \
"connections. Takes effect after the next reboot.\r\n"

#define HELP_DEFINE_SERVER_SECONDARY_NAMESERVER \
"DEFINE SERVER [ETHERNET] SECONDARY NAMESERVER <ip>\r\n" \
"\n" \
"Sets the secondary DNS name server for the Ethernet port. Used when DHCP is not\r\n" \
"being used for automatic configuration. Takes effect after the next reboot.\r\n"

#define HELP_DEFINE_SERVER_WIFI_PSK \
"DEFINE SERVER WIFI PSK <key>\r\n" \
"\n" \
"Set the Pre-Shared Key for the WiFi interface. Takes effect after the next\r\n" \
"reboot.\r\n"

#define HELP_DEFINE_SERVER_WIFI_SSID \
"DEFINE SERVER WIFI SSID <name>\r\n" \
"\n" \
"Set the name of the WiFi network to connect to. Takes effect after the next\r\n" \
"reboot.\r\n"

#define HELP_DEFINE_SERVER_WIFI_SECONDARY_NAMESERVER \
"DEFINE SERVER WIFI SECONDARY NAMESERVER <ip>\r\n" \
"\n" \
"Sets the secondary DNS name server for the WiFi port. Used when DHCP is not\r\n" \
"being used for automatic configuration. Takes effect after the next reboot.\r\n"

#define HELP_SET_PRIVILEGED \
"SET PRIVILEGED\r\n" \
"\n" \
"Enables privileged mode for this port. Requires the privileged password.\r\n"

#define HELP_SET_NOPRIVILEGED \
"SET NOPRIVILEGED\r\n" \
"\n" \
"Disables privileged mode for this port."
        

#define HELP_SET_PORT_BACKWARD \
"SET PORT [<port>] BACKWARD <key>\r\n" \
"\n" \
"Temporarily sets the keypress to switch to the previous session when connected\r\n" \
"to a remote system. For control keys prefix the desired character with ^.\r\n" \
"Symbolic names for many standard special keys (F1, PF3, etc) are available.\r\n" 

#define HELP_SET_PORT_FORWARD \
"SET PORT [<port>] FORWARD <key>\r\n" \
"\n" \
"Temporarily sets the keypress to switch to the next session when connected to a\r\n" \
"remote system. For control keys prefix the desired character with ^. Symbolic\r\n" \
"names for many standard special keys (F1, PF3, etc) are available.\r\n"

#define HELP_SET_PORT_LOCAL \
"SET PORT [<port>] LOCAL <key>\r\n" \
"\n" \
"Temporarily sets the keypress to switch to the Local> prompt when connected to\r\n" \
"a remote system. For control keys prefix the desired character with ^. Symbolic\r\n" \
"names for many standard special keys (F1, PF3, etc) are available.\r\n"

#define HELP_SET_PORT_BREAK_LOCAL \
"SET PORT [<port>] BREAK LOCAL\r\n" \
"\n" \
"Temporarily sets the Break mode to Local on a selected, or the current, port.\r\n" \
"In Local Break mode any Break signal received on the selected port will cause\r\n" \
"that port to enter the Local> prompt for user interaction.\r\n"

#define HELP_SET_PORT_BREAK_REMOTE \
"SET PORT [<port>] BREAK REMOTE\r\n" \
"\n" \
"Temporarily sets the Break mode to Remote on a selected, or the current, port.\r\n" \
"In Remote Break mode any Break signal received on the selected port will be\r\n" \
"forwarded to the system currently being communicated with, if any.\r\n" 

#define HELP_SET_PORT_BREAK_DISABLED \
"SET PORT [<port>] BREAK DISABLED\r\n" \
"\n" \
"Temporarily sets the Break mode to Disabled on a selected, or the current, port.\r\n" \
"In Disabled Break mode any Break signal received on the selected port will be\r\n" \
"ignored.\r\n"

#define HELP_SET_PORT_FLOW_NONE \
"SET PORT [<port>] FLOW NONE\r\n" \
"\n" \
"Temporarily disables flow control on the specified (or current if ommitted)\r\n" \
"port. Only applicable to Serial ports.\r\n"

#define HELP_SET_PORT_FLOW_XONXOFF \
"SET PORT [<port>] FLOW XONXOFF\r\n" \
"\n" \
"Temporarily set the flow control on the specified (or current if ommitted) port\r\n" \
"to XON/XOFF. Only applicable to Serial ports.\r\n"

#define HELP_SET_PORT_FLOW_RTSCTS \
"SET PORT [<port>] FLOW RTSCTS\r\n" \
"\n" \
"Temporarily set the flow control on the specified (or current if ommitted) port\r\n" \
"to RTS/CTS. Only applicable to Serial ports.\r\n"

#define HELP_SET_PORT_FLOW_DTRDSR \
"SET PORT [<port>] FLOW HELP_DEFINE_PORT_FLOW_DTRDSR\r\n" \
"\n" \
"Temporarily et the flow control on the specified (or current if ommitted) port\r\n" \
"to DTR/DSR. Only applicable to Serial ports.\r\n"

#define HELP_SET_PORT_SPEED \
"SET PORT [<port>] SPEED <baud>\r\n" \
"\n" \
"Temporarily set the baud rate of a Serial port. Takes an optional port number,\r\n" \
"otherwise it operates on the current port. Any symmetric baud rate up to 1Mbaud\r\n" \
"is supported. Note that asymmetric baud rates, like 1200/75 are not supported.\r\n" 

#define HELP_SET_PORT_TERMINAL_TYPE \
"SET PORT [<port>] TERMINAL TYPE <type>\r\n" \
"\n" \
"Temporarily sets the terminal type for a port. Used for command line control\r\n" \
"and telnet connections.\r\n"

#define HELP_SHOW_ALL_COMMANDS \
"SHOW ALL COMMANDS\r\n" \
"\n" \
"Show a tree view of all available commands defined in the system.\r\n"

#define HELP_SHOW_ALL_PORTS \
"SHOW ALL PORTS\r\n" \
"\n" \
"List all the available ports and their current status.\r\n"

#define HELP_SHOW_PORT_CHARACTERISTICS \
"SHOW PORT [<port>] [CHARACTERISTICS]\r\n" \
"\n" \
"Show the current settings and status for a port."

#define HELP_SHOW_PORT_STATUS \
"SHOW PORT [<port>] STATUS\r\n" \
"\n" \
"Display the current operating status of a port including any handshaking\r\n" \
"signals available."

#define HELP_SHOW_SERVER \
"SHOW SERVER\r\n" \
"\n" \
"Show the current status and settings of the terminal server."

#define HELP_SHOW_SESSIONS \
"SHOW SESSIONS\r\n" \
"\n" \
"List the currently active sessions associated with the current port."
#endif

        
