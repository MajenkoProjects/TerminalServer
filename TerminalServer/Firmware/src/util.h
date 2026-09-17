#ifndef _UTIL_H
#define _UTIL_H

#include <stdint.h>
#include <stdbool.h>
#include "port.h"

extern char *format_local_switch(int sw, char *buf, int len);
extern int parse_local_switch(const char *sw);
extern int strncasecmp(const char *s1, const char *s2, int n);
extern int strcasecmp(const char *s1, const char *s2);
extern char *ip2str(uint32_t ip, char *str);
extern bool validate_mac(char *mac);
extern bool validate_ip(char *ip);
extern bool validate_netmask(char *ip);
extern int fancy_read(struct port *port, char c, uint16_t *buf, int len);

#endif