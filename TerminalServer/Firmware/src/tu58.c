#include <string.h>

#include "system/fs/sys_fs.h"

#include "tu58.h"

enum tu58_opcode {
    TUOP_NOP = 0,
    TUOP_INIT,
    TUOP_READ,
    TUOP_WRITE,
    // 4 = reserved
    TUOP_POSITION=5,
    // 6 = reserved
    TUOP_DIAGNOSE=7,
    TUOP_GETSTATUS,
    TUOP_SETSTATUS
    // 10-11 = reserved
};

#define TUST_NORMAL 0
#define TUST_RETRIES 1
#define TUST_FAILED_SELFTEST -1
#define TUST_PARTIAL -2
#define TUST_BADUNIT -8
#define TUST_NO_CART -9
#define TUST_WRITEPROT -11
#define TUST_DATA_ERROR -17
#define TUST_SEEK_ERROR -32
#define TUST_MOTOR_STOPPED -33
#define TUST_BAD_OPCODE -48
#define TUST_BADBLOCK -55

enum tu58_state {
    TU58_IDLE,
    TU58_WAIT_COMMAND,
    TU58_START_BOOTSTRAP,
    TU58_RUN_BOOTSTRAP,
    TU58_GET_COMMAND,
    TU58_EXEC_COMMAND,
    TU58_START_ENDPACKET,
    TU58_SEND_PACKET,
    TU58_READ_DATA,
    TU58_GET_WRITE
};

struct tu58_command {
    union {
        uint8_t Val8[14];
        uint16_t Val16[7];
        struct {
            uint8_t flag;
            uint8_t count;
            uint8_t opcode;
            uint8_t modifier;
            uint8_t unit;
            uint8_t switches;
            uint16_t sequence;
            uint16_t bytecount;
            uint16_t blockno;
            uint16_t checksum;            
        } __attribute__((packed));
    } __attribute__((packed));
} __attribute__((packed));

struct tu58_data {
    SYS_FS_HANDLE file[2];
    uint32_t pos[2];
    uint32_t size[2];
    char *filename[2];
    struct tu58_command command;
    uint8_t status;
    enum tu58_state state;
    uint8_t drive;
    uint16_t bufpos;
    uint8_t buffer[128 + 4];
    uint8_t bufsize;
    uint16_t progress;
    enum tu58_state next_state;
};

uint16_t tu58_checksum(uint8_t *data, uint16_t len) {
    uint32_t checksum = 0;
    
    for (int i = 0; i < len; i+=2) {
        checksum += (data[i] | (data[i+1] << 8));
        checksum = (checksum + (checksum >> 16)) & 0xFFFF;
    }
    return checksum;
}

    
void tu58_init(struct port *port) {
    struct tu58_data *data = (struct tu58_data *) malloc(sizeof(struct tu58_data));

    data->file[0] = SYS_FS_HANDLE_INVALID;
    data->file[1] = SYS_FS_HANDLE_INVALID;  
    data->filename[0] = NULL;
    data->filename[1] = NULL;
    data->state = (uint8_t)TU58_IDLE;
    data->drive = 0;
    port->access_data = data;
}

void tu58_process(struct port *port) {    
    struct tu58_data *data = (struct tu58_data *) port->access_data;
    uint32_t space;
    int16_t cs = 0;
    int16_t tmps16;

    switch ((enum tu58_state)data->state) {
        case TU58_IDLE:
            if (port_available(port)) {
                int c = port_read_byte(port);
             //   DBG("[%03o]\r\n", c);
                if (c == 004) {
                    port_write_byte(port, 020); // Continue
                    data->state = TU58_WAIT_COMMAND;
                }

            }
            break;

        case TU58_WAIT_COMMAND:
            if (port_available(port)) {
                int c = port_read_byte(port);
               // DBG("[%03o]\r\n", c);
                switch (c) {
                    case 004: // INIT
                        port_write_byte(port, 020); // Continue
                        data->state = TU58_WAIT_COMMAND;
                        break;
                    case 010: // Bootstrap
                        data->state = TU58_START_BOOTSTRAP;
                        break;
                    case 001: // Data
                        data->buffer[data->bufpos++] = 001;
                        data->state = TU58_GET_WRITE;                        
                        break;
                    case 002: // Command
                        data->command.Val8[0] = c;
                        data->bufpos = 1;
                        data->state = TU58_GET_COMMAND;
                        break;
                }
            }
            break;
        case TU58_START_BOOTSTRAP:
            if (port_available(port)) {
                int c = port_read_byte(port);
                data->drive = c;
               // DBG("Bootstrap drive %d\r\n", data->drive);
                if (data->file[data->drive] == SYS_FS_HANDLE_INVALID) {
                    data->state = TU58_WAIT_COMMAND;
                } else {
                    SYS_FS_FileSeek(data->file[data->drive], 0, SYS_FS_SEEK_SET);
                    data->pos[data->drive] = 0;
                    data->state = TU58_RUN_BOOTSTRAP;
                }
            }
            break;
        case TU58_RUN_BOOTSTRAP:
            space = port_free_space(port);
            if (space > 0) {                    
                if (space > (512 - data->pos[data->drive])) {
                    space = (512 - data->pos[data->drive]);
                }
                if (space > 128) space = 128;

                SYS_FS_FileRead(data->file[data->drive], data->buffer, space);
                port_write(port, data->buffer, space);
                data->pos[data->drive] += space;
                if (data->pos[data->drive] >= 512) {
                    data->state = TU58_WAIT_COMMAND;
                }
            }
            break;
        case TU58_GET_COMMAND:
            if (port_available(port)) {
                int c = port_read_byte(port);
                data->command.Val8[data->bufpos++] = c;
                if (data->bufpos == 14) {
                    data->drive = data->command.unit;
                    data->state = TU58_EXEC_COMMAND;
                }
            }
            break;
        case TU58_EXEC_COMMAND:
            cs = tu58_checksum(data->command.Val8, 12);
            

            tmps16 = data->command.checksum;
            if (tmps16 & 0x8000) {
                tmps16 = ~tmps16;
                tmps16 = 0 - tmps16;
            }

            if (tmps16 != cs) {
                data->state = TU58_WAIT_COMMAND;
                break;
            }
            
            switch (data->command.opcode) {
                case TUOP_NOP:
                    data->status = TUST_NORMAL;
                    data->state = TU58_START_ENDPACKET;
                    break;
                case TUOP_INIT:
                    data->status = TUST_NORMAL;
                    data->state = TU58_START_ENDPACKET;
                    break;                   
                case TUOP_READ:
                    data->progress = 0;
                    data->state = TU58_READ_DATA;
                    break;
                case TUOP_WRITE:
                //    DBG("Write %d bytes to %d\r\n", data->command[data->drive].bytecount, data->command[data->drive].blockno);
                    data->progress = 0;
                    data->bufpos = 0;
//                    data->next_state = 
                    port_write_byte(port, 020);
                    data->state = TU58_WAIT_COMMAND;
                    break;
                    
                case TUOP_POSITION:
                    data->status = TUST_NORMAL;
                    data->state = TU58_START_ENDPACKET;
                    break;                   
                case TUOP_DIAGNOSE:
                    data->status = TUST_NORMAL;
                    data->state = TU58_START_ENDPACKET;
                    break;                   
                case TUOP_GETSTATUS:
                    data->status = TUST_NORMAL;
                    data->state = TU58_START_ENDPACKET;
                    break;                   
                case TUOP_SETSTATUS:       
                    data->status = TUST_NORMAL;
                    data->state = TU58_START_ENDPACKET;
                    break;                   
            }
            
            break;

            
            
        case TU58_READ_DATA:
            if (data->progress >= data->command.bytecount) {
                data->status = TUST_NORMAL;
                data->state = TU58_START_ENDPACKET;
                break;
            }
            space = data->command.bytecount - data->progress;
            if (space > 128) space = 128;
            data->bufpos = 0;            
            data->buffer[0] = 1;
            
            if ((data->command.blockno * 512 + data->progress) > data->size[data->drive]) {
                data->status = TUST_PARTIAL;
                data->state = TU58_START_ENDPACKET;
                break;
            }
            
            SYS_FS_FileSeek(data->file[data->drive], data->command.blockno * 512 + data->progress, SYS_FS_SEEK_SET);
            space = SYS_FS_FileRead(data->file[data->drive], &data->buffer[2], space);
            data->progress += space;
            data->buffer[1] = space;
            
            cs = tu58_checksum(data->buffer, space+2);
        
            data->buffer[space+2] = cs;
            data->buffer[space+3] = cs >> 8;
            data->bufsize = space + 4;
            data->next_state = TU58_READ_DATA;
            data->state = TU58_SEND_PACKET;
            break;
            
        case TU58_SEND_PACKET:
            space = port_free_space(port);
            
            if (space > (data->bufsize - data->bufpos)) {
                space = (data->bufsize - data->bufpos);
            }
            port_write(port, data->buffer + data->bufpos, space);
            data->bufpos += space;
            
            if (data->bufpos >= data->bufsize) {
                data->state = data->next_state;                
            }

            break;          
            
        case TU58_START_ENDPACKET:
            data->buffer[0] = 2;
            data->buffer[1] = 10;
            data->buffer[2] = 0x40;
            data->buffer[3] = data->status;
            data->buffer[4] = data->drive;
            data->buffer[5] = 0;
            data->buffer[6] = data->command.Val8[6];
            data->buffer[7] = data->command.Val8[7];
            data->buffer[8] = data->progress & 0xFF;
            data->buffer[9] = data->progress >> 8;
            data->buffer[10] = 0;
            data->buffer[11] = 0;
            
            cs = tu58_checksum(data->buffer, 12);

            data->buffer[12] = cs & 0xFF;
            data->buffer[13] = cs >> 8;

            data->progress = 0;
            data->bufsize  = 14;
            data->bufpos = 0;
            data->next_state = TU58_WAIT_COMMAND;
            data->state = TU58_SEND_PACKET;
            break;
        case TU58_GET_WRITE:
            if (port_available(port)) {
                int c = port_read_byte(port);    
                data->buffer[data->bufpos++] = c;
                if (data->bufpos >= data->buffer[1] + 4) {
                    SYS_FS_FileSeek(data->file[data->drive], data->command.blockno * 512 + data->progress, SYS_FS_SEEK_SET);
                    SYS_FS_FileWrite(data->file[data->drive], &data->buffer[2], data->buffer[1]);
                    data->progress += data->buffer[1];
                    
                    if (data->progress >= data->command.bytecount) {
                        data->status = TUST_NORMAL;
                        data->state = TU58_START_ENDPACKET;
                        data->next_state = TU58_WAIT_COMMAND;
                    } else {
                        port_write_byte(port, 020);
                        data->state = TU58_WAIT_COMMAND;
                    }
                    data->bufpos = 0;
                }
            }
            break;
    }
}

void tu58_show_info(struct port *port, struct port *target) {
    struct tu58_data *data = (struct tu58_data *) target->access_data;
    for (int i = 0; i < 2; i++) {
        if (data->file[i] == SYS_FS_HANDLE_INVALID) {
            port_printf(port, "   Device %d: None\r\n", i);        
        } else {
            port_printf(port, "   Device %d: %-12s %8d/%8d\r\n", i, data->filename[i], data->pos[i], data->size[i]);
        }
    } 
}

bool tu58_mount(const char *filename, struct port *port, int unit) {
    struct tu58_data *data = (struct tu58_data *) port->access_data;
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
    struct tu58_data *data = (struct tu58_data *) port->access_data;

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
