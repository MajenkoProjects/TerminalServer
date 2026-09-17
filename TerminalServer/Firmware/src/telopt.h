#ifndef _TELOPT_H
#define _TELOPT_H
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
#endif