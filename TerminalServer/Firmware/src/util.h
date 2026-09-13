#ifndef _UTIL_H
#define _UTIL_H

extern void format_local_switch(int sw, char *buf);
extern int strncasecmp(const char *s1, const char *s2, size_t n);
extern char *ip2str(uint32_t ip, char *str);
extern bool validate_mac(char *mac);
extern bool validate_ip(char *ip);
extern bool validate_netmask(char *ip);

#endif