#include <string.h>
#include <ctype.h>
#include "tapes.h"
#include "util.h"
#include "system/fs/sys_fs.h"

const struct tape_functions tapes[] = {
    { .mount = &tu58_mount, .dismount = &tu58_dismount },
};

#define NUM_TAPES (sizeof(tapes) / sizeof(struct tape_functions))


// MOUNT <file> [ON] PORT n DEVICE|UNIT m
COMMAND(tapes_mount) {
    int ano = 0;
    uint32_t portno = 0;
    uint32_t unit = 0;
    SYS_FS_FSTAT fstat;

    if (argc < 5) {
        return ERR_INCOMPLETE;
    }
    
    const char *filename = argv[ano++];
    if (strcasecmp(argv[ano], "on") == 0) {
        ano++;
    }
    
    if (strncasecmp(argv[ano], "PORT", strlen(argv[ano])) != 0) {
        return ERR_UNKNOWN;
    }
    
    ano++;
    
    if (!isdigit(argv[ano][0])) {
        return ERR_UNKNOWN;
    }
    
    portno = strtoul(argv[ano], NULL, 10);
    ano++;
    
    if (strncasecmp(argv[ano], "DEVICE", strlen(argv[ano])) != 0) {
        if (strncasecmp(argv[ano], "UNIT", strlen(argv[ano])) != 0) {
            return ERR_UNKNOWN;
        }
    }
    
    ano++;
    
    if (!isdigit(argv[ano][0])) {
        return ERR_UNKNOWN;
    }
    
    unit = strtoul(argv[ano], NULL, 10);
    
    
    struct port *tp = get_port_by_number(portno);
    
    if (!tp) return ERR_BADPORT;
    if (tp->access < ACCESS_TU58) return ERR_BADPORT;
    
    
    fstat.lfname = NULL;
    fstat.lfsize = 0;
    
    if (SYS_FS_FileStat(filename, &fstat) == SYS_FS_RES_FAILURE) {
        return ERR_FILENOTFOUND;
    }
    
    port_printf(port, "Mounting %s on %s unit %d\r\n", filename, tp->name, unit);
    
    int tapeno = tp->access - ACCESS_TU58;
    
    if (tapeno >= NUM_TAPES) {
        return ERR_UNKNOWN;
    }
    
    
    if (tapes[tapeno].mount) {
        if (tapes[tapeno].mount(filename, tp, unit) == false) {
            return ERR_MOUNTFAIL;
        }
    }
    return ERR_OK;
}