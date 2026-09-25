#include <ctype.h>

#include "driver/enc28j60/drv_enc28j60.h"
#include "network.h"
#include "definitions.h"
#include "settings.h"
#include "util.h"


struct ethernet_settings ethernet_settings;
struct wifi_settings wifi_settings;



/* ENC 600 Driver Configuration */
const DRV_ENC28J60_Configuration drvEnc28j60InitData[] = {
    {
        .txDescriptors =        DRV_ENC28J60_MAC_TX_DESCRIPTORS_IDX0,
        .rxDescriptors =        DRV_ENC28J60_MAC_RX_DESCRIPTORS_IDX0,
        .rxDescBufferSize =     DRV_ENC28J60_MAX_RX_BUFFER_IDX0,
        .rxBufferSize =         DRV_ENC28J60_RX_BUFFER_SIZE_IDX0,
        .maxFrameSize =         DRV_ENC28J60_MAX_FRAME_SIZE_IDX0,
        .spiSetup.chipSelect =  DRV_ENC28J60_SPI_CS_IDX0,
    },
};
TCPIP_NETWORK_CONFIG TCPIP_HOSTS_CONFIGURATION[] = {

    /*** Network Configuration Index 0 ***/
    {
        .interface = "eth0",
        .hostName = system_settings.nodename, // TCPIP_NETWORK_DEFAULT_HOST_NAME_IDX0,
        .macAddr = ethernet_settings.macaddr, // TCPIP_NETWORK_DEFAULT_MAC_ADDR_IDX0,
        .ipAddr = ethernet_settings.ip, //TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX0,
        .ipMask = ethernet_settings.netmask, //TCPIP_NETWORK_DEFAULT_IP_MASK_IDX0,
        .gateway = ethernet_settings.gateway, //TCPIP_NETWORK_DEFAULT_GATEWAY_IDX0,
        .priDNS = ethernet_settings.pridns, //TCPIP_NETWORK_DEFAULT_DNS_IDX0,
        .secondDNS = ethernet_settings.secdns, //TCPIP_NETWORK_DEFAULT_SECOND_DNS_IDX0,
        .powerMode = TCPIP_NETWORK_DEFAULT_POWER_MODE_IDX0,
        .startFlags = TCPIP_NETWORK_DEFAULT_INTERFACE_FLAGS_IDX0,
        .pMacObject = &TCPIP_NETWORK_DEFAULT_MAC_DRIVER_IDX0,

    },
    /*
    {
        .interface = "wlan0",
        .hostName = system_settings.nodename, // TCPIP_NETWORK_DEFAULT_HOST_NAME_IDX0,
        .macAddr = wifi_settings.macaddr, // TCPIP_NETWORK_DEFAULT_MAC_ADDR_IDX0,
        .ipAddr = wifi_settings.ip, //TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX0,
        .ipMask = wifi_settings.netmask, //TCPIP_NETWORK_DEFAULT_IP_MASK_IDX0,
        .gateway = wifi_settings.gateway, //TCPIP_NETWORK_DEFAULT_GATEWAY_IDX0,
        .priDNS = wifi_settings.pridns, //TCPIP_NETWORK_DEFAULT_DNS_IDX0,
        .secondDNS = wifi_settings.secdns, //TCPIP_NETWORK_DEFAULT_SECOND_DNS_IDX0,
        .powerMode = TCPIP_NETWORK_DEFAULT_POWER_MODE_IDX0,
        .startFlags = TCPIP_NETWORK_DEFAULT_INTERFACE_FLAGS_IDX0,
        .pMacObject = &ESP32_MAC_Object,

    },
    */
};

// <editor-fold defaultstate="collapsed" desc="TCP/IP Stack Initialization Data">
// *****************************************************************************
// *****************************************************************************
// Section: TCPIP Data
// *****************************************************************************
// *****************************************************************************
const TCPIP_ARP_MODULE_CONFIG tcpipARPInitData = { 
    .cacheEntries       = TCPIP_ARP_CACHE_ENTRIES,     
    .deleteOld          = TCPIP_ARP_CACHE_DELETE_OLD,    
    .entrySolvedTmo     = TCPIP_ARP_CACHE_SOLVED_ENTRY_TMO, 
    .entryPendingTmo    = TCPIP_ARP_CACHE_PENDING_ENTRY_TMO, 
    .entryRetryTmo      = TCPIP_ARP_CACHE_PENDING_RETRY_TMO, 
    .permQuota          = TCPIP_ARP_CACHE_PERMANENT_QUOTA, 
    .purgeThres         = TCPIP_ARP_CACHE_PURGE_THRESHOLD, 
    .purgeQuanta        = TCPIP_ARP_CACHE_PURGE_QUANTA, 
    .retries            = TCPIP_ARP_CACHE_ENTRY_RETRIES, 
    .gratProbeCount     = TCPIP_ARP_GRATUITOUS_PROBE_COUNT,
};
const TCPIP_UDP_MODULE_CONFIG tcpipUDPInitData = {
    .nSockets       = TCPIP_UDP_MAX_SOCKETS,
    .sktTxBuffSize  = TCPIP_UDP_SOCKET_DEFAULT_TX_SIZE, 
};
const TCPIP_TCP_MODULE_CONFIG tcpipTCPInitData = {
    .nSockets       = TCPIP_TCP_MAX_SOCKETS,
    .sktTxBuffSize  = TCPIP_TCP_SOCKET_DEFAULT_TX_SIZE, 
    .sktRxBuffSize  = TCPIP_TCP_SOCKET_DEFAULT_RX_SIZE,
};
const TCPIP_DHCP_MODULE_CONFIG tcpipDHCPInitData = {     
    .dhcpEnable     = false,   
    .dhcpTmo        = TCPIP_DHCP_TIMEOUT,
    .dhcpCliPort    = TCPIP_DHCP_CLIENT_CONNECT_PORT,
    .dhcpSrvPort    = TCPIP_DHCP_SERVER_LISTEN_PORT,

};

const TCPIP_ICMP_MODULE_CONFIG tcpipICMPInitData = {
    0
};
const TCPIP_DNS_CLIENT_MODULE_CONFIG tcpipDNSClientInitData = {
    .deleteOldLease         = TCPIP_DNS_CLIENT_DELETE_OLD_ENTRIES,
    .cacheEntries           = TCPIP_DNS_CLIENT_CACHE_ENTRIES,
    .entrySolvedTmo         = TCPIP_DNS_CLIENT_CACHE_ENTRY_TMO,    
    .nIPv4Entries  = TCPIP_DNS_CLIENT_CACHE_PER_IPV4_ADDRESS,
    .ipAddressType       = TCPIP_DNS_CLIENT_ADDRESS_TYPE,
    .nIPv6Entries  = TCPIP_DNS_CLIENT_CACHE_PER_IPV6_ADDRESS,
};
const TCPIP_IPV4_MODULE_CONFIG  tcpipIPv4InitData = {
    .arpEntries = TCPIP_IPV4_ARP_SLOTS, 
};
TCPIP_STACK_HEAP_EXTERNAL_CONFIG tcpipHeapConfig = {
    .heapType = TCPIP_STACK_HEAP_TYPE_EXTERNAL,
    .heapFlags = TCPIP_STACK_HEAP_USE_FLAGS,
    .heapUsage = TCPIP_STACK_HEAP_USAGE_CONFIG,
    .malloc_fnc = TCPIP_STACK_MALLOC_FUNC,
    .free_fnc = TCPIP_STACK_FREE_FUNC,
    .calloc_fnc = TCPIP_STACK_CALLOC_FUNC,
};


const size_t TCPIP_HOSTS_CONFIGURATION_SIZE = sizeof (TCPIP_HOSTS_CONFIGURATION) / sizeof (*TCPIP_HOSTS_CONFIGURATION);

const TCPIP_STACK_MODULE_CONFIG TCPIP_STACK_MODULE_CONFIG_TBL [] = {
    {TCPIP_MODULE_IPV4,             &tcpipIPv4InitData},
    {TCPIP_MODULE_ICMP,             0},                             // TCPIP_MODULE_ICMP
    {TCPIP_MODULE_ARP,              &tcpipARPInitData},             // TCPIP_MODULE_ARP
    {TCPIP_MODULE_UDP,              &tcpipUDPInitData},             // TCPIP_MODULE_UDP
    {TCPIP_MODULE_TCP,              &tcpipTCPInitData},             // TCPIP_MODULE_TCP
    {TCPIP_MODULE_DHCP_CLIENT,      &tcpipDHCPInitData},            // TCPIP_MODULE_DHCP_CLIENT
    {TCPIP_MODULE_DNS_CLIENT,       &tcpipDNSClientInitData},       // TCPIP_MODULE_DNS_CLIENT
    { TCPIP_MODULE_MANAGER,         &tcpipHeapConfig },             // TCPIP_MODULE_MANAGER
    {TCPIP_MODULE_MAC_ENCJ60,       &drvEnc28j60InitData},          // TCPIP_MODULE_MAC_ENCJ60
};

const size_t TCPIP_STACK_MODULE_CONFIG_TBL_SIZE = sizeof (TCPIP_STACK_MODULE_CONFIG_TBL) / sizeof (*TCPIP_STACK_MODULE_CONFIG_TBL);


SYS_MODULE_OBJ TCPIP_STACK_Init(void) {
    TCPIP_STACK_INIT    tcpipInit;

    TCPIP_HOSTS_CONFIGURATION[0].hostName = system_settings.nodename;
    TCPIP_HOSTS_CONFIGURATION[0].macAddr = ethernet_settings.macaddr;
    TCPIP_HOSTS_CONFIGURATION[0].ipAddr = ethernet_settings.ip;
    TCPIP_HOSTS_CONFIGURATION[0].ipMask = ethernet_settings.netmask;
    TCPIP_HOSTS_CONFIGURATION[0].gateway = ethernet_settings.gateway;
    TCPIP_HOSTS_CONFIGURATION[0].priDNS = ethernet_settings.pridns;
    TCPIP_HOSTS_CONFIGURATION[0].secondDNS = ethernet_settings.secdns;
    TCPIP_HOSTS_CONFIGURATION[0]. startFlags = ethernet_settings.flags;   
    
    (void)memset(&tcpipInit, 0, sizeof(tcpipInit));
    tcpipInit.pNetConf = TCPIP_HOSTS_CONFIGURATION;
    tcpipInit.nNets = TCPIP_HOSTS_CONFIGURATION_SIZE;
    tcpipInit.pModConfig = TCPIP_STACK_MODULE_CONFIG_TBL;
    tcpipInit.nModules = TCPIP_STACK_MODULE_CONFIG_TBL_SIZE;
    tcpipInit.initCback = NULL; //&TCPIP_STACK_InitCallback;

    return TCPIP_STACK_Initialize(0, &tcpipInit);
}


#include "net_pres/pres/net_pres_enc_glue.h"

static const NET_PRES_TransportObject netPresTransObject0SS = {
    .fpOpen        = (NET_PRES_TransOpen)TCPIP_TCP_ServerOpen,
    .fpLocalBind         = (NET_PRES_TransBind)TCPIP_TCP_Bind,
    .fpRemoteBind        = (NET_PRES_TransBind)TCPIP_TCP_RemoteBind,
    .fpOptionGet         = (NET_PRES_TransOption)TCPIP_TCP_OptionsGet,
    .fpOptionSet         = (NET_PRES_TransOption)TCPIP_TCP_OptionsSet,
    .fpIsConnected       = (NET_PRES_TransBool)TCPIP_TCP_IsConnected,
    .fpWasReset          = (NET_PRES_TransBool)TCPIP_TCP_WasReset,
    .fpWasDisconnected   = (NET_PRES_TransBool)TCPIP_TCP_WasDisconnected,
    .fpDisconnect        = (NET_PRES_TransBool)TCPIP_TCP_Disconnect,
    .fpConnect           = (NET_PRES_TransBool)TCPIP_TCP_Connect,
    .fpClose             = (NET_PRES_TransClose)TCPIP_TCP_Close,
    .fpSocketInfoGet     = (NET_PRES_TransSocketInfoGet)TCPIP_TCP_SocketInfoGet,
    .fpFlush             = (NET_PRES_TransBool)TCPIP_TCP_Flush,
    .fpPeek              = (NET_PRES_TransPeek)TCPIP_TCP_ArrayPeek,
    .fpDiscard           = (NET_PRES_TransDiscard)TCPIP_TCP_Discard,
    .fpHandlerRegister   = (NET_PRES_TransHandlerRegister)TCPIP_TCP_SignalHandlerRegister,
    .fpHandlerDeregister = (NET_PRES_TransSignalHandlerDeregister)TCPIP_TCP_SignalHandlerDeregister,
    .fpRead              = (NET_PRES_TransRead)TCPIP_TCP_ArrayGet,
    .fpWrite             = (NET_PRES_TransWrite)TCPIP_TCP_ArrayPut,
    .fpReadyToRead       = (NET_PRES_TransReady)TCPIP_TCP_GetIsReady,
    .fpReadyToWrite      = (NET_PRES_TransReady)TCPIP_TCP_PutIsReady,
    .fpIsPortDefaultSecure = (NET_PRES_TransIsPortDefaultSecured)TCPIP_Helper_TCPSecurePortGet,
};
static const NET_PRES_TransportObject netPresTransObject0SC = {
    .fpOpen        = (NET_PRES_TransOpen)TCPIP_TCP_ClientOpen,
    .fpLocalBind         = (NET_PRES_TransBind)TCPIP_TCP_Bind,
    .fpRemoteBind        = (NET_PRES_TransBind)TCPIP_TCP_RemoteBind,
    .fpOptionGet         = (NET_PRES_TransOption)TCPIP_TCP_OptionsGet,
    .fpOptionSet         = (NET_PRES_TransOption)TCPIP_TCP_OptionsSet,
    .fpIsConnected       = (NET_PRES_TransBool)TCPIP_TCP_IsConnected,
    .fpWasReset          = (NET_PRES_TransBool)TCPIP_TCP_WasReset,
    .fpWasDisconnected   = (NET_PRES_TransBool)TCPIP_TCP_WasDisconnected,
    .fpDisconnect        = (NET_PRES_TransBool)TCPIP_TCP_Disconnect,
    .fpConnect           = (NET_PRES_TransBool)TCPIP_TCP_Connect,
    .fpClose             = (NET_PRES_TransClose)TCPIP_TCP_Close,
    .fpSocketInfoGet     = (NET_PRES_TransSocketInfoGet)TCPIP_TCP_SocketInfoGet,
    .fpFlush             = (NET_PRES_TransBool)TCPIP_TCP_Flush,
    .fpPeek              = (NET_PRES_TransPeek)TCPIP_TCP_ArrayPeek,
    .fpDiscard           = (NET_PRES_TransDiscard)TCPIP_TCP_Discard,
    .fpHandlerRegister   = (NET_PRES_TransHandlerRegister)TCPIP_TCP_SignalHandlerRegister,
    .fpHandlerDeregister = (NET_PRES_TransSignalHandlerDeregister)TCPIP_TCP_SignalHandlerDeregister,
    .fpRead              = (NET_PRES_TransRead)TCPIP_TCP_ArrayGet,
    .fpWrite             = (NET_PRES_TransWrite)TCPIP_TCP_ArrayPut,
    .fpReadyToRead       = (NET_PRES_TransReady)TCPIP_TCP_GetIsReady,
    .fpReadyToWrite      = (NET_PRES_TransReady)TCPIP_TCP_PutIsReady,
    .fpIsPortDefaultSecure = (NET_PRES_TransIsPortDefaultSecured)TCPIP_Helper_TCPSecurePortGet,
};
static const NET_PRES_TransportObject netPresTransObject0DS = {
    .fpOpen        = (NET_PRES_TransOpen)TCPIP_UDP_ServerOpen,
    .fpLocalBind         = (NET_PRES_TransBind)TCPIP_UDP_Bind,
    .fpRemoteBind        = (NET_PRES_TransBind)TCPIP_UDP_RemoteBind,
    .fpOptionGet         = (NET_PRES_TransOption)TCPIP_UDP_OptionsGet,
    .fpOptionSet         = (NET_PRES_TransOption)TCPIP_UDP_OptionsSet,
    .fpIsConnected       = (NET_PRES_TransBool)TCPIP_UDP_IsConnected,
    .fpWasReset          = NULL,
    .fpWasDisconnected   = NULL,
    .fpDisconnect        = (NET_PRES_TransBool)TCPIP_UDP_Disconnect,
    .fpConnect          = NULL,
    .fpClose             = (NET_PRES_TransClose)TCPIP_UDP_Close,
    .fpSocketInfoGet     = (NET_PRES_TransSocketInfoGet)TCPIP_UDP_SocketInfoGet,
    .fpFlush             = (NET_PRES_TransBool)TCPIP_UDP_Flush,
    .fpPeek              = NULL,
    .fpDiscard           = (NET_PRES_TransDiscard)TCPIP_UDP_Discard,
    .fpHandlerRegister   = (NET_PRES_TransHandlerRegister)TCPIP_UDP_SignalHandlerRegister,
    .fpHandlerDeregister = (NET_PRES_TransSignalHandlerDeregister)TCPIP_UDP_SignalHandlerDeregister,
    .fpRead              = (NET_PRES_TransRead)TCPIP_UDP_ArrayGet,
    .fpWrite             = (NET_PRES_TransWrite)TCPIP_UDP_ArrayPut,
    .fpReadyToRead       = (NET_PRES_TransReady)TCPIP_UDP_GetIsReady,
    .fpReadyToWrite      = (NET_PRES_TransReady)TCPIP_UDP_PutIsReady,
    .fpIsPortDefaultSecure = (NET_PRES_TransIsPortDefaultSecured)TCPIP_Helper_UDPSecurePortGet,
};
static const NET_PRES_TransportObject netPresTransObject0DC = {
    .fpOpen        = (NET_PRES_TransOpen)TCPIP_UDP_ClientOpen,
    .fpLocalBind         = (NET_PRES_TransBind)TCPIP_UDP_Bind,
    .fpRemoteBind        = (NET_PRES_TransBind)TCPIP_UDP_RemoteBind,
    .fpOptionGet         = (NET_PRES_TransOption)TCPIP_UDP_OptionsGet,
    .fpOptionSet         = (NET_PRES_TransOption)TCPIP_UDP_OptionsSet,
    .fpIsConnected       = (NET_PRES_TransBool)TCPIP_UDP_IsConnected,
    .fpWasReset          = NULL,
    .fpWasDisconnected   = NULL,
    .fpDisconnect        = (NET_PRES_TransBool)TCPIP_UDP_Disconnect,
    .fpConnect          = NULL,
    .fpClose             = (NET_PRES_TransClose)TCPIP_UDP_Close,
    .fpSocketInfoGet     = (NET_PRES_TransSocketInfoGet)TCPIP_UDP_SocketInfoGet,
    .fpFlush             = (NET_PRES_TransBool)TCPIP_UDP_Flush,
    .fpPeek              = NULL,
    .fpDiscard           = (NET_PRES_TransDiscard)TCPIP_UDP_Discard,
    .fpHandlerRegister   = (NET_PRES_TransHandlerRegister)TCPIP_UDP_SignalHandlerRegister,
    .fpHandlerDeregister = (NET_PRES_TransSignalHandlerDeregister)TCPIP_UDP_SignalHandlerDeregister,
    .fpRead              = (NET_PRES_TransRead)TCPIP_UDP_ArrayGet,
    .fpWrite             = (NET_PRES_TransWrite)TCPIP_UDP_ArrayPut,
    .fpReadyToRead       = (NET_PRES_TransReady)TCPIP_UDP_GetIsReady,
    .fpReadyToWrite      = (NET_PRES_TransReady)TCPIP_UDP_PutIsReady,
    .fpIsPortDefaultSecure = (NET_PRES_TransIsPortDefaultSecured)TCPIP_Helper_UDPSecurePortGet,
};
static const NET_PRES_INST_DATA netPresCfgs[] = {                                                          
                                                                                                                                                 
    {                                                                                                                                            
        .pTransObject_ss = &netPresTransObject0SS,                                                                                               
        .pTransObject_sc = &netPresTransObject0SC,                                                                                               
        .pTransObject_ds = &netPresTransObject0DS,                                                                                               
        .pTransObject_dc = &netPresTransObject0DC,                                                                                               
        .pProvObject_ss = NULL,                                                                                                                  
        .pProvObject_sc = NULL,                                                                                                                  
        .pProvObject_ds = NULL,                                                                                                                  
        .pProvObject_dc = NULL,                                                                                                                  
    },                                                                                                                                           
                                                                                                                                                 
};  
static const NET_PRES_INIT_DATA netPresInitData = {
    .numLayers = sizeof(netPresCfgs) / sizeof(NET_PRES_INST_DATA),
    .pInitData = netPresCfgs
};

void ethernet_boot() {
    port_printf(CONSOLE, "MAC Address: %s\r\n", ethernet_settings.macaddr);
    CONSOLE->fn_flush(CONSOLE);
    port_printf(CONSOLE, "Initializing network...");
    CONSOLE->fn_flush(CONSOLE);
    sysObj.netPres = NET_PRES_Initialize(0, (SYS_MODULE_INIT*)&netPresInitData);
    sysObj.tcpip = TCPIP_STACK_Init();

    const TCPIP_NET_HANDLE *handle = TCPIP_STACK_NetHandleGet("eth0");
 
    int tries = ethernet_settings.timeout * 2;
    while ((!TCPIP_STACK_NetIsLinked(handle)) && (tries > 0)) {
            vTaskDelay(500);
            port_printf(CONSOLE, ".");
            CONSOLE->fn_flush(CONSOLE);
            tries--;
    } 
    port_printf(CONSOLE, "\r\n");
    CONSOLE->fn_flush(CONSOLE);

    if (!TCPIP_STACK_NetIsLinked(handle)) {
        port_printf(CONSOLE, "No network link detected. Giving up.\r\n");
        CONSOLE->fn_flush(CONSOLE);
        return;
    }

    // Wait for DHCP address
    uint32_t ts = xTaskGetTickCount();
    if (ethernet_settings.flags & TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON) {
        port_printf(CONSOLE, "Waiting for DHCP address...");
        CONSOLE->fn_flush(CONSOLE);
        while (!TCPIP_STACK_NetIsReady(handle)) {
            vTaskDelay(500);
            if (xTaskGetTickCount() - ts >= 500) {
                ts = xTaskGetTickCount();
                port_printf(CONSOLE, ".");            
                CONSOLE->fn_flush(CONSOLE);
            }
        }
        port_printf(CONSOLE, "\r\n");
        CONSOLE->fn_flush(CONSOLE);
    }
    
    char ip[16];
    ip2str(TCPIP_STACK_NetAddress(handle), ip);
    
    char gw[16];
    ip2str(TCPIP_STACK_NetAddressGateway(handle), gw);
    port_printf(CONSOLE, "IP Address: %-15s Gateway: %s\r\n", ip, gw);
    CONSOLE->fn_flush(CONSOLE);

#if 0
//    MDNSD_ERR_CODE
    switch (TCPIP_MDNS_ServiceRegister(
            handle, 
            "Telnet on Terminal Server", 
             "_telnet._tcp.local",
            23,
            (uint8_t *)"",
            1,
            NULL,
            NULL
            )) {        
        case MDNSD_SUCCESS:
            port_printf(CONSOLE, "mDNS: Bound and running\r\n");
            break;
        case MDNSD_ERR_BUSY:
            port_printf(CONSOLE, "mDNS: Already in use by another service\r\n");
            break;
        case MDNSD_ERR_CONFLICT:
            port_printf(CONSOLE, "mDNS: Name conflict detected\r\n");
            break;
        case MDNSD_ERR_INVAL:
            port_printf(CONSOLE, "mDNS: Invalid parameter specified\r\n");
            break;
    
    }
    CONSOLE->fn_flush(CONSOLE);
#endif    
    /*
    for (struct port *scan = ports; scan; scan = scan->next) {
        if ((scan->access = ACCESS_REMOTE) || (scan->access = ACCESS_DYNAMIC)) {
            switch(TCPIP_MDNS_ServiceRegister(
                handle, 
                scan->name,
                "_serial._tcp.local",
                3000 + scan->no,
                (uint8_t *)"",
                1,
                NULL,
                NULL
            )) {
        
        case MDNSD_SUCCESS:
            port_printf(CONSOLE, "mDNS: Bound and running\r\n");
            break;
        case MDNSD_ERR_BUSY:
            port_printf(CONSOLE, "mDNS: Already in use by another service\r\n");
            break;
        case MDNSD_ERR_CONFLICT:
            port_printf(CONSOLE, "mDNS: Name conflict detected\r\n");
            break;
        case MDNSD_ERR_INVAL:
            port_printf(CONSOLE, "mDNS: Invalid parameter specified\r\n");
            break;
    
    }

        }
    }
    */
}

void ethernet_load_setting(uint8_t module, uint8_t parameter, uint8_t index, uint8_t length, uint8_t *data) {
    switch (parameter) {
        case SETTINGS_ETHERNET_MAC:
            strncpy(ethernet_settings.macaddr, (char *)data, 17);
            break;
        case SETTINGS_ETHERNET_IP:
            strncpy(ethernet_settings.ip, (char *)data, length);
            break;
        case SETTINGS_ETHERNET_NETMASK:
            strncpy(ethernet_settings.netmask, (char *)data, length);
            break;
        case SETTINGS_ETHERNET_GATEWAY:
            strncpy(ethernet_settings.gateway, (char *)data, length);
            break;
        case SETTINGS_ETHERNET_PRIDNS:
            strncpy(ethernet_settings.pridns, (char *)data, length);
            break;
        case SETTINGS_ETHERNET_SECDNS:
            strncpy(ethernet_settings.secdns, (char *)data, length);
            break;
        case SETTINGS_ETHERNET_FLAGS:
            ethernet_settings.flags = (*(uint16_t *)data) | TCPIP_NETWORK_CONFIG_MULTICAST_ON;
            break;
            
        case SETTINGS_WIFI_MAC:
            strncpy(wifi_settings.macaddr, (char *)data, 17);
            break;
        case SETTINGS_WIFI_IP:
            strncpy(wifi_settings.ip, (char *)data, length);
            break;
        case SETTINGS_WIFI_NETMASK:
            strncpy(wifi_settings.netmask, (char *)data, length);
            break;
        case SETTINGS_WIFI_GATEWAY:
            strncpy(wifi_settings.gateway, (char *)data, length);
            break;
        case SETTINGS_WIFI_PRIDNS:
            strncpy(wifi_settings.pridns, (char *)data, length);
            break;
        case SETTINGS_WIFI_SECDNS:
            strncpy(wifi_settings.secdns, (char *)data, length);
            break;
        case SETTINGS_WIFI_FLAGS:
            wifi_settings.flags = (*(uint16_t *)data) | TCPIP_NETWORK_CONFIG_MULTICAST_ON;
            break;
        case SETTINGS_WIFI_SSID:
            memset(wifi_settings.ssid, 0, 33);
            memcpy(wifi_settings.ssid, data, length < 33 ? length : 32);
            break;
        case SETTINGS_WIFI_PSK:
            memset(wifi_settings.psk, 0, 64);
            memcpy(wifi_settings.psk, data, length < 64 ? length : 63);
            break;
    }
}

void ethernet_init_defaults() {
    snprintf(ethernet_settings.macaddr, 19, "DE:AD:BE:EF:%02X:%02X", (DEVCFG3bits.USERID >> 8) & 0xFF, DEVCFG3bits.USERID & 0xFF);
    strcpy(ethernet_settings.ip, "192.168.1.100");
    strcpy(ethernet_settings.netmask, "255.255.255.0");
    strcpy(ethernet_settings.gateway, "192.168.1.1");
    strcpy(ethernet_settings.pridns, "8.8.8.8");
    strcpy(ethernet_settings.secdns, "8.8.4.4");
    ethernet_settings.flags = (TCPIP_NETWORK_CONFIG_MULTICAST_ON | TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON | TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC);
    ethernet_settings.timeout = 10;

    snprintf(wifi_settings.macaddr, 19, "BA:BE:FA:CE:%02X:%02X", (DEVCFG3bits.USERID >> 8) & 0xFF, DEVCFG3bits.USERID & 0xFF);
    strcpy(wifi_settings.ip, "192.168.2.100");
    strcpy(wifi_settings.netmask, "255.255.255.0");
    strcpy(wifi_settings.gateway, "192.168.2.1");
    strcpy(wifi_settings.pridns, "8.8.8.8");
    strcpy(wifi_settings.secdns, "8.8.4.4");
    wifi_settings.flags = (TCPIP_NETWORK_CONFIG_MULTICAST_ON | TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON | TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC);
    wifi_settings.timeout = 10;
    memset(wifi_settings.ssid, 0, 33);
    memset(wifi_settings.psk, 0, 64);

}

void print_network_settings(struct port *port) {
    port_printf(port, "Ethernet settings:\r\n");
    port_printf(port, "   MAC Address: %s\r\n", ethernet_settings.macaddr);
    port_printf(port, "Configured:\r\n");
    port_printf(port, "   IP Address:  %-15s   Netmask:       %-15s\r\n", ethernet_settings.ip, ethernet_settings.netmask);
    port_printf(port, "   Gateway:     %-15s   DNS:           %s/%s\r\n", ethernet_settings.gateway, ethernet_settings.pridns, ethernet_settings.secdns);
    if (ethernet_settings.flags & TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON) {
        port_printf(port, "   DHCP:        Enabled\r\n");
        port_printf(port, "DHCP:\r\n");
        const TCPIP_NET_HANDLE *handle = TCPIP_STACK_NetHandleGet("eth0");

        
        char ip[16];
        ip2str(TCPIP_STACK_NetAddress(handle), ip);
        
        char gw[16];
        ip2str(TCPIP_STACK_NetAddressGateway(handle), gw);

        char dns1[16];
        ip2str(TCPIP_STACK_NetAddressDnsPrimary(handle), dns1);

        char dns2[16];
        ip2str(TCPIP_STACK_NetAddressDnsSecond(handle), dns2);
        
        port_printf(port, "   IP Address:  %-15s   Gateway:       %s\r\n", ip, gw);
        port_printf(port, "   DNS Primary: %-15s   DNS Secondary: %s\r\n", dns1, dns2);
    } else {
        port_printf(port, "   DHCP:        Disabled\r\n");
    }

    port_printf(port, "\nWiFi settings:\r\n");
    port_printf(port, "   MAC Address: %s\r\n", wifi_settings.macaddr);
    port_printf(port, "Configured:\r\n");
    port_printf(port, "   IP Address:  %-15s   Netmask:       %-15s\r\n", wifi_settings.ip, wifi_settings.netmask);
    port_printf(port, "   Gateway:     %-15s   DNS:           %s/%s\r\n", wifi_settings.gateway, wifi_settings.pridns, wifi_settings.secdns);
    if (wifi_settings.flags & TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON) {
        port_printf(port, "   DHCP:        Enabled\r\n");
        port_printf(port, "DHCP:\r\n");
/* TODO: Enable this when the wifi MAC code is written
        const TCPIP_NET_HANDLE *handle = TCPIP_STACK_NetHandleGet("wlan0");

        
        char ip[16];
        ip2str(TCPIP_STACK_NetAddress(handle), ip);
        
        char gw[16];
        ip2str(TCPIP_STACK_NetAddressGateway(handle), gw);

        char dns1[16];
        ip2str(TCPIP_STACK_NetAddressDnsPrimary(handle), dns1);

        char dns2[16];
        ip2str(TCPIP_STACK_NetAddressDnsSecond(handle), dns2);
        
        port_printf(port, "   IP Address:  %-15s   Gateway:       %s\r\n", ip, gw);
        port_printf(port, "   DNS Primary: %-15s   DNS Secondary: %s\r\n", dns1, dns2);
 */
    } else {
        port_printf(port, "   DHCP:        Disabled\r\n");
    }
    port_printf(port, "   SSID: %s\r\n", wifi_settings.ssid[0] == 0 ? "None" : wifi_settings.ssid);
    port_printf(port, "   PSK: %s\r\n", wifi_settings.psk[0] == 0 ? "Unset" : "Set");
}

COMMAND(ethernet_define_mac_address) {
    if (argc == 0) {
        return ERR_INCOMPLETE;
    }
    
    char *mac = argv[0];
    if (!validate_mac(mac)) return ERR_INVALID;
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_MAC, 0, 17, (uint8_t *)mac);
    return ERR_OK;
    
}
COMMAND(ethernet_define_ip) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_IP, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK;
}
COMMAND(ethernet_define_subnet) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_netmask(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_NETMASK, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK;
}
COMMAND(ethernet_define_gateway) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_GATEWAY, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(ethernet_define_pridns) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_PRIDNS, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(ethernet_define_secdns) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_SECDNS, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(ethernet_define_dhcp_enabled) {
    uint16_t flags = TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON | TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC;
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_FLAGS, 0, 2, (uint8_t *)&flags);
    return ERR_OK;
}
COMMAND(ethernet_define_dhcp_disabled) {
    uint16_t flags = TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC;
    setting_set(MODULE_ETHERNET, SETTINGS_ETHERNET_FLAGS, 0, 2, (uint8_t *)&flags);
    return ERR_OK;
}

COMMAND(wifi_define_mac_address) {
    if (argc == 0) {
        return ERR_INCOMPLETE;
    }
    
    char *mac = argv[0];
    if (!validate_mac(mac)) return ERR_INVALID;
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_MAC, 0, 17, (uint8_t *)mac);
    return ERR_OK;
    
}
COMMAND(wifi_define_ip) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_IP, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK;
}
COMMAND(wifi_define_subnet) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_netmask(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_NETMASK, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK;
}
COMMAND(wifi_define_gateway) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_GATEWAY, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(wifi_define_pridns) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_PRIDNS, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(wifi_define_secdns) {
    if (argv == 0) return ERR_INCOMPLETE;
    
    char *ip = argv[0];
    if (!validate_ip(ip)) return ERR_INVALID;
    
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_SECDNS, 0, strlen(ip), (uint8_t *)ip);    
    return ERR_OK; 
}
COMMAND(wifi_define_dhcp_enabled) {
    uint16_t flags = TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON | TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC;
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_FLAGS, 0, 2, (uint8_t *)&flags);
    return ERR_OK;
}
COMMAND(wifi_define_dhcp_disabled) {
    uint16_t flags = TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC;
    setting_set(MODULE_ETHERNET, SETTINGS_WIFI_FLAGS, 0, 2, (uint8_t *)&flags);
    return ERR_OK;
}
COMMAND(wifi_define_ssid) {
    if (argc != 1) return ERR_INCOMPLETE;
    int length = strlen(argv[0]);
    if (length > 32) length = 32;

    if (strcasecmp(argv[0], "none") == 0) {
        setting_set(MODULE_ETHERNET, SETTINGS_WIFI_SSID, 0, 0, NULL);
    } else {
        setting_set(MODULE_ETHERNET, SETTINGS_WIFI_SSID, 0, length, (uint8_t *)argv[0]);
    }
    return ERR_OK;
}
COMMAND(wifi_define_psk) {
    if (argc != 1) return ERR_INCOMPLETE;
    int length = strlen(argv[0]);
    if (length > 63) length = 63;
    if (strcasecmp(argv[0], "none") == 0) {
        setting_set(MODULE_ETHERNET, SETTINGS_WIFI_SSID, 0, 0, NULL);
    } else {
        setting_set(MODULE_ETHERNET, SETTINGS_WIFI_PSK, 0, length, (uint8_t *)argv[0]);
    }
    return ERR_OK;    
}

//void add_mac(int index, TCPIP_MAC_ADDR *mac) {
//    TCPIP_HOSTS_CONFIGURATION[index].pMacObject->MAC_RxFilterHashTableEntrySet(NULL, mac);
//}