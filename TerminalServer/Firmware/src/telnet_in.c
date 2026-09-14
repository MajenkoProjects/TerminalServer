#include <errno.h>
#include "app.h"
#include "telnet_in.h"
#include "port.h"
#include "session.h"

#define TELOPT_BINARY       0
#define TELOPT_ECHO         1
#define TELOPT_RECONN       2
#define TELOPT_SUPPRESS_GA  3
#define TELOPT_AMSN         4
#define TELOPT_STATUS       5
#define TELOPT_TM           6
#define TELOPT_RCTE         7
#define TELOPT_OLW          8
#define TELOPT_OPS          9
#define TELOPT_OCRD         10
#define TELOPT_OHTS         11
#define TELOPT_OHTD         12
#define TELOPT_OFFD         13
#define TELOPT_OVTS         14
#define TELOPT_OVTD         15
#define TELOPT_OLFD         16
#define TELOPT_EXTEND_ASCII 17
#define TELOPT_LOGOUT       18
#define TELOPT_BYTE_MACRO   19
#define TELOPT_DET          20
#define TELOPT_SUPDUP       21
#define TELOPT_SUPDUP_OUT   22
#define TELOPT_SEND_LOC     23
#define TELOPT_TTYPE        24
#define TELOPT_EOR          25
#define TELOPT_TACACS       26
#define TELOPT_OUTPUT_MARK  27
#define TELOPT_LOCNO        28
#define TELOPT_3270_REGIME  29
#define TELOPT_X3_PAD       30
#define TELOPT_NAWS         31
#define TELOPT_SPEED        32
#define TELOPT_FLOW         33
#define TELOPT_LINEMODE     34
#define TELOPT_X_DISP_LOC   35
#define TELOPT_ENV_OPT      36
#define TELOPT_AUTH_OPT     37
#define TELOPT_ENC_OPT      38
#define TELOPT_NEW_ENV_OPT  39
#define TELOPT_TN3270E      40
#define TELOPT_XAUTH        41




#define TELOPT_SE           240
#define TELOPT_NOP          241
#define TELOPT_DM           242
#define TELOPT_BREAK        243
#define TELOPT_IP           244
#define TELOPT_AO           245
#define TELOPT_AYT          246
#define TELOPT_EC           247
#define TELOPT_EL           248
#define TELOPT_GA           249
#define TELOPT_SB           250
#define TELOPT_WILL         251
#define TELOPT_WONT         252
#define TELOPT_DO           253
#define TELOPT_DONT         254
#define TELOPT_IAC          255

#define SB_IS               0
#define SB_SEND             1

const char IAC_WILL_SUPPRESS_GA[3] = {TELOPT_IAC, TELOPT_WILL, TELOPT_SUPPRESS_GA};
const char IAC_DO_SUPPRESS_GA[3] = {TELOPT_IAC, TELOPT_DO, TELOPT_SUPPRESS_GA};
const char IAC_DONT_LINEMODE[3] = {TELOPT_IAC, TELOPT_DONT, TELOPT_LINEMODE};
const char IAC_DONT_ECHO[3] = {TELOPT_IAC, TELOPT_DONT, TELOPT_ECHO};
const char IAC_WONT_LINEMODE[3] = {TELOPT_IAC, TELOPT_WONT, TELOPT_LINEMODE};
const char IAC_WILL_ECHO[3] = {TELOPT_IAC, TELOPT_WILL, TELOPT_ECHO};
const char IAC_DO_NAWS[3] = {TELOPT_IAC, TELOPT_DO, TELOPT_NAWS};
const char IAC_DO_TTYPE[3] = {TELOPT_IAC, TELOPT_DO, TELOPT_TTYPE};
const char IAC_SB_TTYPE_SEND[6] = {TELOPT_IAC, TELOPT_SB, TELOPT_TTYPE, SB_SEND, TELOPT_IAC, TELOPT_SE};

static int telnet_fd = -1;
TaskHandle_t telnet_in_thread_handle;

static void telnet_in_close_port(struct port *port) {
    struct telnet_in_data *data = (struct telnet_in_data *)port->port_data;
    vTaskDelay(100);
    data->queue_close = true;
}

static void telnet_in_thread(void *args) {

  //  vTaskDelay(1000);
    struct sockaddr_in sa;
    telnet_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (telnet_fd < 0) {
        debugf("Socket failed: %d\r\n", errno);
        return;
    }
    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = 23; 
    if (bind(telnet_fd, (struct sockaddr*)&sa, sizeof(struct sockaddr_in)) < 0) {
        debugf("Bind failed: %d\r\n", errno);
        return;
    }
    if (listen(telnet_fd, 5) < 0) {
        debugf("Listen failed: %d\r\n", errno);
        return;
    }
   // debugf("Telnet bound and listening on port 23\r\n");
    
    while (1) {
        for (struct port *scan = ports; scan; scan = scan->next) {
            if (scan->type == PORT_TELNET_IN) {
                struct telnet_in_data *data = (struct telnet_in_data *)scan->port_data;

                if (data->queue_close) {
                    closesocket(data->fd);
                    delete_port(scan);
                    continue;
                }
 //               if (NET_PRES_SocketWasDisconnected(TCPIP_BSD_PresSocket(data->fd))) {
  //                  port_printf(CONSOLE, "Close 3\r\n");
        //            close_port(scan);
//                    delete_port(scan);
   //             } else {

                    if (cb_free(&(scan->read_buffer)) > 0) {
                        uint8_t b;
                        int r = recv(data->fd, (char *)&b, 1, 0);
                        
                        
                        if (r == 1) {
                            
                            if (data->iac_sb) {
                                switch (b) {
                                    case TELOPT_IAC:
                                        if (data->iac_sb_iac) {
                                            data->iac_sub[data->iac_sub_pos++] = b;
                                            data->iac_sub[data->iac_sub_pos] = 0;
                                            if (data->iac_sub_pos >= 40) {
                                                data->iac_sb = false;
                                                data->iac_pos = 0;    
                                            }                                        
                                        } else {
                                            data->iac_sb_iac = true;
                                        }
                                        break;
                                    case TELOPT_SE:
                                        switch (data->iac_sub[0]) {
                                            case TELOPT_NAWS:
                                                scan->columns = (data->iac_sub[1] << 8) | data->iac_sub[2];
                                                scan->lines = (data->iac_sub[3] << 8) | data->iac_sub[4];
                                                break;
                                            case TELOPT_TTYPE:
                                                for (int i = 2; i < data->iac_sub_pos; i++) {
                                                    int x = i - 2;
                                                    if (x < 16) {
                                                        scan->ttype[x] = data->iac_sub[i];
                                                        scan->ttype[x+1] = 0;
                                                    }
                                                }
                                                break;
                                            default:
                                                port_printf(CONSOLE, "SUB %d\r\n", data->iac_sub[0]);
                                        }
                                        data->iac_sb = false;
                                        data->iac_pos = 0;
                                        break;
                                    default:
                                        data->iac_sub[data->iac_sub_pos++] = b;
                                        data->iac_sub[data->iac_sub_pos] = 0;
                                        if (data->iac_sub_pos >= 40) {
                                        data->iac_sb = false;
                                            data->iac_pos = 0;    
                                        }
                                        break;
                                }
                            } else {
                                switch (data->iac_pos) {
                                    case 0: 
                                        if (b == TELOPT_IAC) {
                                            data->iac[data->iac_pos++] = TELOPT_IAC;
                                        } else {
                                            cb_write(&(scan->read_buffer), b);
                                        }
                                        break;

                                    case 1:
                                        switch (b) {
                                            case TELOPT_IAC:
                                                cb_write(&(scan->read_buffer), b);
                                                data->iac_pos = 0;                                       
                                                break;
                                            case TELOPT_SB:
                                                data->iac_sb = true;
                                                data->iac_sub_pos = 0;
                                                break;
                                            case TELOPT_BREAK:
                                                data->iac_pos = 0;
                                                switch (scan->breakmode) {
                                                    case BREAK_LOCAL:
                                                        port_printf(scan, "+++ OUT OF CHEESE +++\r\n\n");
                                                        port_printf(scan, "Local>");
                                                        scan->mode = MODE_LOCAL;
                                                        break;
                                                    case BREAK_REMOTE:
                                                        if (scan->active_session) {
                                                            scan->active_session->target->send_break = true;
                                                        }
                                                        break;
                                                    default:
                                                        break;
                                                }
                                                break;
                                            default:
                                                data->iac[data->iac_pos++] = b;
                                        }
                                        break;

                                    case 2:

                                        switch (b) {
                                            default:
                                                data->iac[data->iac_pos++] = b;
                                                switch (data->iac[1]) {
                                                    case TELOPT_WILL:
                                                        switch (data->iac[2]) {
                                                            case TELOPT_LINEMODE:
                                                                while (send(data->fd, IAC_DONT_LINEMODE, 3, 0) != 3);
                                                                while (send(data->fd, IAC_DONT_ECHO, 3, 0) != 3);
                                                                while (send(data->fd, IAC_WILL_ECHO, 3, 0) != 3);
                                                                break;
                                                            case TELOPT_TTYPE:
                                                                while (send(data->fd, IAC_DO_TTYPE, 3, 0) != 3);
                                                                while (send(data->fd, IAC_SB_TTYPE_SEND, 6, 0) != 6);
                                                                break;
                                                            case TELOPT_NAWS:
                                                                while (send(data->fd, IAC_DO_NAWS, 3, 0) != 3);
                                                                break;
                                                            default:
                                                                //port_printf(CONSOLE, "WILL %d\r\n", data->iac[2]);
                                                                break;

                                                        }
                                                        break;
                                                    case TELOPT_WONT:
                                                        break;
                                                    case TELOPT_DO:
                                                        switch (data->iac[2]) {
                                                            case TELOPT_SUPPRESS_GA:
                                                                while (send(data->fd, IAC_WILL_SUPPRESS_GA, 3, 0) != 3);
                                                                while (send(data->fd, IAC_DO_SUPPRESS_GA, 3, 0) != 3);
                                                                break;
                                                            default:
                                                                break;
                                                        }
                                                        break;
                                                    case TELOPT_DONT:
                                                        break;
                                                    default:
                                                        break;
                                                }
                                                data->iac_pos = 0;
                                        }
                                }
                            }
                        } else {
                            if (errno != EWOULDBLOCK) {
                                closesocket(data->fd);
                                close_port(scan);
                                delete_port(scan);
                                continue;
                            }
                        }
                    }

                    int num = cb_available(&scan->write_buffer);
                    if (num > 0) {
                        if (num > CIRCULAR_BUFFER_SIZE) num = CIRCULAR_BUFFER_SIZE;
                        char tmp[CIRCULAR_BUFFER_SIZE];
                        for (int i = 0; i < num; i++) {
                            tmp[i] = cb_read(&scan->write_buffer);
                        }
                        int count = 0;
                        while (num > 0) {
                            int sent = send(data->fd, &tmp[count], num, 0);
                            if (sent != num) {
                                if (errno != EWOULDBLOCK) {
                                    closesocket(data->fd);
                                    delete_port(scan);
                                    close_port(scan);
                                    break;
                                }
                            }
                            if (sent > 0) {
                                count += sent;
                                num -= sent;
                            }
                        }
//                        vTaskDelay(10);
                    }
                //}
            }
            
        }
        
        struct sockaddr_in isa;
        int slen = sizeof(struct sockaddr_in);
        int fd = accept(telnet_fd, (struct sockaddr *)&isa, &slen);
        if (fd > 0) {    
            struct telnet_in_data *td = malloc(sizeof(struct telnet_in_data));
            memset(td, 0, sizeof(struct telnet_in_data));
            td->iac_pos = 0;
            memcpy(&td->sin, &isa, slen);
            td->fd = fd;
            struct port *p = add_port(PORT_TELNET_IN, td);
            p->mode = MODE_PREGREET;
            p->fn_close = &telnet_in_close_port;
            p->fn_show_detail = &print_telnet_in_info;
            snprintf(p->name, 9, "Telnt%d", p->no);
            p->name[8] = 0;
            p->breakmode = BREAK_LOCAL;
        }   
    }
}

void telnet_in_initialize() {
    (void) xTaskCreate(
           (TaskFunction_t) telnet_in_thread,
           "Telnet_In",
           1024,   
           NULL,
           1U ,
           &telnet_in_thread_handle);
}

void print_telnet_in_info(struct port *port, struct port *target) {
    struct telnet_in_data *data = (struct telnet_in_data *)target->port_data;
    char ip[16];
    snprintf(ip, 16, "%d.%d.%d.%d\r\n", 
            data->sin.sin_addr.S_un.S_un_b.s_b1,
            data->sin.sin_addr.S_un.S_un_b.s_b2,
            data->sin.sin_addr.S_un.S_un_b.s_b3,
            data->sin.sin_addr.S_un.S_un_b.s_b4
            );
    port_printf(port, "   Remote Address:  %15s\r\n", ip);
}