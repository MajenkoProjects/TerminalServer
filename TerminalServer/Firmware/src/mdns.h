#ifndef _MDNS_H
#define	_MDNS_H

extern void mdns_init();
extern void mdns_advertise_service(const char *name, const char *type, const char *protocol, uint16_t port);
extern void mdns_tasks();
#endif	
