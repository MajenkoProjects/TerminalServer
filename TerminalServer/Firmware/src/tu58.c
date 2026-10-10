#include <string.h>

#include "system/fs/sys_fs.h"

#include "tu58.h"

struct tu58_data {
    SYS_FS_HANDLE file[2];          // 8
    uint32_t pos[2];                // 8
    uint32_t size[2];               // 8
    char *filename[2];              // 8
} __attribute__((packed));          // 32/64

void tu58_init(struct port *port) {
    struct tu58_data *data = (struct tu58_data *) port->misc;

    data->file[0] = SYS_FS_HANDLE_INVALID;
    data->file[1] = SYS_FS_HANDLE_INVALID;
    data->filename[0] = NULL;
    data->filename[1] = NULL;
}

void tu58_process(struct port *port) {    
}

void tu58_show_info(struct port *port, struct port *target) {
    struct tu58_data *data = (struct tu58_data *) target->misc;
    for (int i = 0; i < 2; i++) {
        if (data->file[i] == SYS_FS_HANDLE_INVALID) {
            port_printf(port, "   Device %d: None\r\n", i);        
        } else {
            port_printf(port, "   Device %d: %-12s %8d/%8d\r\n", i, data->filename[i], data->pos[i], data->size[i]);
        }
    } 
}

bool tu58_mount(const char *filename, struct port *port, int unit) {
    struct tu58_data *data = (struct tu58_data *) port->misc;
    SYS_FS_FSTAT fstat;

    if ((unit < 0) || (unit > 1)) {
        return false;
    }

    if (data->file[unit] != SYS_FS_HANDLE_INVALID) {
        tu58_dismount(port, unit);
    }

    fstat.lfname = NULL;
    fstat.lfsize = 0;
    
    if (SYS_FS_FileStat(filename, &fstat) == SYS_FS_RES_FAILURE) {
        return false;
    }

    if (fstat.fsize > (65536 * 512)) {
        return false;
    }    

    data->size[unit] = fstat.fsize;

    
    data->file[unit] = SYS_FS_FileOpen(filename, SYS_FS_FILE_OPEN_READ_PLUS);
    if (data->file[unit] == SYS_FS_HANDLE_INVALID) {
        return false;
    }


    data->filename[unit] = strdup(filename);    
    data->pos[unit] = 0;
    return true;
}

bool tu58_dismount(struct port *port, int unit) {
    struct tu58_data *data = (struct tu58_data *) port->misc;

    if ((unit < 0) || (unit > 1)) {
        return false;
    }
    
    if (data->file[unit] != SYS_FS_HANDLE_INVALID) {
        SYS_FS_FileClose(data->file[unit]);
    }
    
    if (data->filename[unit]) {
        free(data->filename[unit]);
    }
    
    return false;
}
