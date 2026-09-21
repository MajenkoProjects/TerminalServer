#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "util.h"
#include "port.h"
#include "ttype.h"


char *format_local_switch(int sw, char *buf, int len) {
    if (sw == LOCAL_SWITCH_NONE) {
        strncpy(buf, "None", len);
        return buf;
    }
    
    if (sw < ' ') {
        snprintf(buf, len, "^%c", sw + '@');
        return buf;
    }
    
    if (IS_SPECIAL(sw)) {
        snprintf(buf, len, "%s", key_names[sw & 0xFF]);
        return buf;
    }
    if ((sw >= '!') && (sw <= '~')) {
        snprintf(buf, len, "%c", sw);
        return buf;
    }
    snprintf(buf, len, "Error");
    return buf;
}

// None -> -1
// ^x -> 0-31
// x 33 -> 255
// <key> -> 0x8000+


int parse_local_switch(const char *sw) {
    switch (strlen(sw)) {
        case 0:
            return LOCAL_SWITCH_ERROR;
        case 1:
            if ((sw[0] >= '!') && (sw[0] <= 126)) return sw[0];
            return LOCAL_SWITCH_ERROR;
        case 2:
            if (sw[0] == '^') {
                if ((sw[1] >= '@') && (toupper(sw[1]) <= 'Z')) {
                    return toupper(sw[1]) - '@';
                }
                return LOCAL_SWITCH_ERROR;
            }
            break;
        default:
            break;
    }
    if (strcasecmp("none", sw) == 0) return LOCAL_SWITCH_NONE;
    for (int i = 0; i < NUM_KEYS; i++) {
        if (strcasecmp(sw, key_names[i]) == 0) {
            return SPECIAL_KEY | i;
        }
    }
    return LOCAL_SWITCH_ERROR;        
}

int strncasecmp(const char *s1, const char *s2, int n) {
   if (n == 0)
     return 0;
 
   while((n-- != 0)
     && (tolower(*(unsigned char *) s1) ==
         tolower(*(unsigned char *) s2))) {
     if (n == 0 || *s1 == '\0' || *s2 == '\0')
       return 0;
     s1++;
     s2++;
   }
 
   return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
 }

int strcasecmp(const char *s1, const char *s2) {
	const char *ptr1 = (const char *)s1;
	const char *ptr2 = (const char *)s2;
	while (tolower(*ptr1) == tolower(*ptr2++)) {
		if (*ptr1++ == '\0') {
			return (0);
        }
    }
	return (tolower(*ptr1) - tolower(*--ptr2));
}



char *ip2str(uint32_t ip, char *str) {
    snprintf(str, 16, "%d.%d.%d.%d",
            ip & 0xFF,
            (ip >> 8) & 0xFF,
            (ip >> 16) & 0xFF,
            (ip >> 24) & 0xFF
            );
    return str;
}

// Check a MAC address for validity and sanitise it.
bool validate_mac(char *mac) {
    if (strlen(mac) != 17) return false;
    
    for (int i = 0; i < 18; i++) {
        mac[i] = toupper(mac[i]);
    }

    if (mac[2] == '-') mac[2] = ':';
    if (mac[5] == '-') mac[5] = ':';
    if (mac[8] == '-') mac[8] = ':';
    if (mac[11] == '-') mac[11] = ':';
    if (mac[14] == '-') mac[14] = ':';
    
    
    if (!isxdigit(mac[0])) return false;
    if (!isxdigit(mac[1])) return false;
    if (mac[2] != ':') return false;
    if (!isxdigit(mac[3])) return false;
    if (!isxdigit(mac[4])) return false;
    if (mac[5] != ':') return false;
    if (!isxdigit(mac[6])) return false;
    if (!isxdigit(mac[7])) return false;
    if (mac[8] != ':') return false;
    if (!isxdigit(mac[9])) return false;
    if (!isxdigit(mac[10])) return false;
    if (mac[11] != ':') return false;
    if (!isxdigit(mac[12])) return false;
    if (!isxdigit(mac[13])) return false;
    if (mac[14] != ':') return false;
    if (!isxdigit(mac[15])) return false;
    if (!isxdigit(mac[16])) return false;
    return true;
}

bool validate_ip(char *ip) {
    int len = strlen(ip);
    if (len < 7) return false;
    if (len > 15) return false;
    
    for (int i = 0; i < len; i++) {
        if ((ip[i] != '.') && !isdigit(ip[i])) return false;
    }
    
    char tmp[16];
    strcpy(tmp, ip);
    char *ptr = NULL;
    char *s1 = strtok_r(tmp, ".", &ptr);
    char *s2 = strtok_r(NULL, ".", &ptr);
    char *s3 = strtok_r(NULL, ".", &ptr);
    char *s4 = strtok_r(NULL, ".", &ptr);
    
    if (!s4) return false;
    
    uint32_t b1 = strtoul(s1, NULL, 10);
    uint32_t b2 = strtoul(s2, NULL, 10);
    uint32_t b3 = strtoul(s3, NULL, 10);
    uint32_t b4 = strtoul(s4, NULL, 10);
    
    if (b1 > 255) return false;
    if (b2 > 255) return false;
    if (b3 > 255) return false;
    if (b4 > 255) return false;
    return true;
}
#define TOPBIT (1 << 31)
bool validate_netmask(char *ip) {
    int len = strlen(ip);
    if (len < 7) return false;
    if (len > 15) return false;
    
    for (int i = 0; i < len; i++) {
        if ((ip[i] != '.') && !isdigit(ip[i])) return false;
    }
    
    char tmp[16];
    strcpy(tmp, ip);
    char *ptr = NULL;
    char *s1 = strtok_r(tmp, ".", &ptr);
    char *s2 = strtok_r(NULL, ".", &ptr);
    char *s3 = strtok_r(NULL, ".", &ptr);
    char *s4 = strtok_r(NULL, ".", &ptr);
    
    if (!s4) return false;
    
    uint32_t b1 = strtoul(s1, NULL, 10);
    uint32_t b2 = strtoul(s2, NULL, 10);
    uint32_t b3 = strtoul(s3, NULL, 10);
    uint32_t b4 = strtoul(s4, NULL, 10);

    if (b1 > 255) return false;
    if (b2 > 255) return false;
    if (b3 > 255) return false;
    if (b4 > 255) return false;

    
    uint32_t val = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
    
    bool have1 = false;
    bool have0 = false;
    for (int i = 0; i < 32; i++) {
        if (!have1) {
            if ((val & TOPBIT) == TOPBIT) {
                have1 = true;
            } else {
                return false;
            }
        } else {
            if ((val & TOPBIT) == 0) {
                have0 = true;
            } else if (have0) {
                return false;
            }
        }
        val <<= 1;
    }
    
    
    return true;
}

int fancy_read(struct port *port, char c, uint16_t *buf, int len) {
    //port_printf(CONSOLE, "[%d]", c);
    port->keybuf[port->keybuf_pos++] = c;
    port->keybuf[port->keybuf_pos] = 0;

    if (port->keybuf_pos == len) {
        memcpy(buf, port->keybuf, len);
        port->keybuf_pos = 0;
        port->keybuf[0] = 0;
        return len;
    }
    
    int num_found = 0;
    int exact = -1;
    for (int i = 0; i < NUM_KEYS; i++) {
        if (port->tinfo->keys[i]) {
            if (strncmp(port->tinfo->keys[i], port->keybuf, port->keybuf_pos) == 0) {
                num_found++;
                if (strcmp(port->tinfo->keys[i], port->keybuf) == 0) {
                    exact = i;
                    break;
                }
            }
        }
    }
    
    if (num_found == 0) {
        for (int i = 0; i < port->keybuf_pos; i++) {
            buf[i] = port->keybuf[i];
        }
        int r = port->keybuf_pos;
        port->keybuf_pos = 0;
        port->keybuf[0] = 0;
        return r;
    }
    
    if (num_found > 1) {
        return 0;
    }
    
    if (exact == -1) {
        return 0;
    }
    
    buf[0] = SPECIAL_KEY | exact;
    buf[1] = 0;
    port->keybuf_pos = 0;
    port->keybuf[0] = 0;    
   // port_printf(port, "Special key %s\r\n", key_names[buf[0] & 0xFF]);
    return 1;
}