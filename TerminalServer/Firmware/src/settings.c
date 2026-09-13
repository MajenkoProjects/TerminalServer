#include <string.h>
#include <stdio.h>

#include "app.h"
#include "settings.h"
#include "uart.h"
#include "usb.h"
#include "network.h"

#include "config/default/peripheral/i2c/master/plib_i2c2_master.h"

#define IP(A,B,C,D) ((A << 24) | (B << 16) | (C << 8) | D)



struct system_settings system_settings;


struct settings_callbacks {
    void(*cbLoad)(uint8_t module, uint8_t parameter, uint8_t index, uint8_t len, uint8_t *data);
    bool(*cbGetName)(uint8_t module, uint8_t parameter, uint8_t index, char *buf, uint8_t *len);
    bool(*cbRenderSetting)(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data, char *buf, uint8_t *len);

};

static struct settings_callbacks callbacks[] = {
    { &system_load_setting, NULL, NULL },
    { &uart_load_setting, &uart_get_setting_name, &uart_render_setting },
    { &usb_load_setting, NULL, NULL },
    { NULL, NULL, NULL }, // SD
    { &ethernet_load_setting, NULL, NULL },
    { NULL, NULL, NULL }, // WIFI
    { &port_load_setting, NULL, NULL }, // Global port settings
};

#define NUM_MODULES (sizeof(callbacks) / sizeof(struct settings_callbacks))

void system_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data) {
    switch (parameter) {
        case SETTINGS_SYSTEM_NAME:
            memset(system_settings.nodename, 0, 33);
            if (length > 32) length = 32;
            strncpy(system_settings.nodename, (char *)data, length);
            break;
    }
}



void load_settings() {
    struct setting setting;
    uint8_t set_addr[2];
    uint8_t i2c_addr = 0x50;
    
    I2C2_Initialize();
    
    for (int addr = 0; addr < 131072; addr+=128) {
        if (addr >= 65536) {
            int a = addr - 65536;
            set_addr[0] = (a >> 8) & 0xFF;
            set_addr[1] = a & 0xFF;
            i2c_addr = 0x54;
        } else {
            int a = addr;
            set_addr[0] = (a >> 8) & 0xFF;
            set_addr[1] = a & 0xFF;
            i2c_addr = 0x50;
        }
        I2C2_Write(i2c_addr, set_addr, 2);
        while (I2C2_IsBusy());
        I2C2_Read(i2c_addr, (uint8_t *)&setting, 3); //sizeof(struct setting));
        while (I2C2_IsBusy());
        
        if (setting.module < NUM_MODULES) {
            I2C2_Read(i2c_addr, (uint8_t *)&setting + 3, sizeof(struct setting) - 3);
            while (I2C2_IsBusy());
            if (callbacks[setting.module].cbLoad != NULL) {
                callbacks[setting.module].cbLoad(setting.module, setting.parameter, setting.index, setting.length, setting.data);
            }
        }
    }
}

void settings_erase() {
    uint8_t buf[130];
    for (int i = 0; i < 130; i++) {
        buf[i] = 0xFF;
    }
    
    for (int i = 0; i < 65536; i+= 128) {
        buf[0] = (i >> 8) &0xFF;
        buf[1] = i & 0xFF;
        I2C2_Write(0x50, buf, 130);
        while (I2C2_IsBusy());
        I2C2_Write(0x54, buf, 130);
        while (I2C2_IsBusy());
    }
}

void setting_set(uint8_t module, uint8_t parameter, uint8_t index, uint8_t len, uint8_t *data) {
    int first_unused = -1;
    
    uint8_t set_addr[2];
    uint8_t i2c_addr = 0x50;
    struct setting setting;

    for (int addr = 0; addr < 131072; addr+=128) {
        if (addr >= 65536) {
            int a = addr - 65536;
            set_addr[0] = (a >> 8) & 0xFF;
            set_addr[1] = a & 0xFF;
            i2c_addr = 0x54;
        } else {
            int a = addr;
            set_addr[0] = (a >> 8) & 0xFF;
            set_addr[1] = a & 0xFF;
            i2c_addr = 0x50;
        }
        I2C2_Write(i2c_addr, set_addr, 2);
        while (I2C2_IsBusy());
        I2C2_Read(i2c_addr, (uint8_t *)&setting, 3); //sizeof(struct setting));
        while (I2C2_IsBusy());
        
        if ((setting.module >= NUM_MODULES) && (first_unused == -1)) {
            first_unused = addr;
        }

        if (setting.module == module && setting.parameter == parameter && setting.index == index) {
            I2C2_Read(i2c_addr, (uint8_t *)&setting + 3, sizeof(struct setting) - 3);
            while (I2C2_IsBusy());
            memcpy(setting.data, data, len);
            setting.length = len;
            uint8_t buf[130];
            buf[0] = set_addr[0];
            buf[1] = set_addr[1];
            memcpy(&buf[2], &setting, 128);
            I2C2_Write(i2c_addr, buf, 130);
            while (I2C2_IsBusy());
            return;
        }
    }

    if (first_unused > -1) {

        if (first_unused >= 65536) {
            int a = first_unused - 65536;
            set_addr[0] = (a >> 8) & 0xFF;
            set_addr[1] = a & 0xFF;
            i2c_addr = 0x54;
        } else {
            int a = first_unused;
            set_addr[0] = (a >> 8) & 0xFF;
            set_addr[1] = a & 0xFF;
            i2c_addr = 0x50;
        }     
        setting.module = module;
        setting.parameter = parameter;
        setting.index = index;
        memcpy(setting.data, data, len);
        setting.length = len;
        uint8_t buf[130];
        buf[0] = set_addr[0];
        buf[1] = set_addr[1];
        memcpy(&buf[2], &setting, 128);
        I2C2_Write(i2c_addr, buf, 130);
        while (I2C2_IsBusy());
    }
}

void settings_dump(struct port *port) {
    char name[128];
    uint8_t len = 128;
    struct setting setting;
    uint8_t set_addr[2];
    uint8_t i2c_addr = 0x50;
    
    I2C2_Initialize();
    
    for (int addr = 0; addr < 131072; addr+=128) {
        if (addr >= 65536) {
            int a = addr - 65536;
            set_addr[0] = (a >> 8) & 0xFF;
            set_addr[1] = a & 0xFF;
            i2c_addr = 0x54;
        } else {
            int a = addr;
            set_addr[0] = (a >> 8) & 0xFF;
            set_addr[1] = a & 0xFF;
            i2c_addr = 0x50;
        }
        I2C2_Write(i2c_addr, set_addr, 2);
        while (I2C2_IsBusy());
        I2C2_Read(i2c_addr, (uint8_t *)&setting, 3); //sizeof(struct setting));
        while (I2C2_IsBusy());
        
        if (setting.module < NUM_MODULES) {
            I2C2_Read(i2c_addr, (uint8_t *)&setting + 3, sizeof(struct setting) - 3);
            while (I2C2_IsBusy());
            if (callbacks[setting.module].cbGetName != NULL) {
                len = 128;
                if (callbacks[setting.module].cbGetName(setting.module, setting.parameter, setting.index, name, &len)) {
                    port_printf(port, "%s=", name);
                }
                len = 128;
                if (callbacks[setting.module].cbRenderSetting(setting.module, setting.parameter, setting.index, setting.length, setting.data, name, &len)) {
                    port_printf(port, "%s", name);
                }
                port_printf(port, "\r\n");
            }
        }
    }
}

void system_init_defaults() {
    strcpy(system_settings.nodename, "muppet");
}

void print_system_settings(struct port *port) {
    port_printf(port, "Server Name:    %s\r\n", system_settings.nodename);
}

COMMAND(system_define_name) {
    if (argc == 0) {
        return ERR_INCOMPLETE;
    }
    
    if (argc > 1) {
        return ERR_SPACES;
    }
    
    if (strlen(argv[0]) > 32) {
        return ERR_TOOLONG;
    }
    
    setting_set(MODULE_SYSTEM, SETTINGS_SYSTEM_NAME, 0, strlen(argv[0]), (uint8_t *)argv[0]);
    return ERR_OK;
}