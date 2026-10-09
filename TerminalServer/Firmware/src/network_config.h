#ifndef _NETWORK_CONFIG_H
#define	_NETWORK_CONFIG_H

#include "config/default/driver/enc28j60/drv_enc28j60.h"

#define ETHERNET_ENABLED
#define WIFI_ENABLED


/* ENC 600 Driver Configuration */
const DRV_ENC28J60_Configuration drvEnc28j60InitData[] = {
    {
        .txDescriptors =        DRV_ENC28J60_MAC_TX_DESCRIPTORS_IDX0,
        .rxDescriptors =        DRV_ENC28J60_MAC_RX_DESCRIPTORS_IDX0,
        .rxDescBufferSize =     DRV_ENC28J60_MAX_RX_BUFFER_IDX0,
        .rxBufferSize =         DRV_ENC28J60_RX_BUFFER_SIZE_IDX0,
        .maxFrameSize =         DRV_ENC28J60_MAX_FRAME_SIZE_IDX0,
        .spiSetup.baudRateInHz = ETHERNET_SPI_BITRATE,
        .spiSetup.clockPolarity = DRV_SPI_CLOCK_POLARITY_IDLE_LOW,
        .spiSetup.clockPhase = DRV_SPI_CLOCK_PHASE_VALID_TRAILING_EDGE,
        .spiSetup.dataBits =    DRV_SPI_DATA_BITS_8,
        .spiSetup.chipSelect =  DRV_ENC28J60_SPI_CS_IDX0,
        .intPin =               ENC_INT_PIN
    },
};
TCPIP_NETWORK_CONFIG TCPIP_HOSTS_CONFIGURATION[] = {

    /*** Network Configuration Index 0 ***/
#ifdef ETHERNET_ENABLED
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
#endif
#ifdef WIFI_ENABLED
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
        .pMacObject = &wifiMacObject,

    },
#endif    
};



/*** FTP Server Initialization Data ***/
const TCPIP_FTP_MODULE_CONFIG tcpipFTPInitData =
{ 
    .cmdPort            = TCPIP_FTPS_COMMAND_LISTEN_PORT, 
    .dataPort           = TCPIP_FTPS_DATA_LISTEN_PORT, 
    .nConnections       = TCPIP_FTP_MAX_CONNECTIONS,
    .dataSktTxBuffSize  = TCPIP_FTP_DATA_SKT_TX_BUFF_SIZE,
    .dataSktRxBuffSize  = TCPIP_FTP_DATA_SKT_RX_BUFF_SIZE,
    .mountPath          = TCPIP_FTP_MOUNT_POINT,
    .userName           = TCPIP_FTP_USER_NAME,
    .password           = TCPIP_FTP_PASSWORD,
};

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
    {TCPIP_MODULE_FTP_SERVER,       &tcpipFTPInitData},             // TCPIP_MODULE_FTP
    { TCPIP_MODULE_MANAGER,         &tcpipHeapConfig },             // TCPIP_MODULE_MANAGER
#ifdef ETHERNET_ENABLED
    {TCPIP_MODULE_MAC_ENCJ60,       &drvEnc28j60InitData},          // TCPIP_MODULE_MAC_ENCJ60
#endif
#ifdef WIFI_ENABLED
    {TCPIP_MODULE_MAC_EXTERNAL,     0},
#endif
};

const size_t TCPIP_STACK_MODULE_CONFIG_TBL_SIZE = sizeof (TCPIP_STACK_MODULE_CONFIG_TBL) / sizeof (*TCPIP_STACK_MODULE_CONFIG_TBL);


SYS_MODULE_OBJ TCPIP_STACK_Init(void) {
    TCPIP_STACK_INIT    tcpipInit;

#ifdef ETHERNET_ENABLED
    TCPIP_HOSTS_CONFIGURATION[0].hostName = system_settings.nodename;
    TCPIP_HOSTS_CONFIGURATION[0].macAddr = ethernet_settings.macaddr;
    TCPIP_HOSTS_CONFIGURATION[0].ipAddr = ethernet_settings.ip;
    TCPIP_HOSTS_CONFIGURATION[0].ipMask = ethernet_settings.netmask;
    TCPIP_HOSTS_CONFIGURATION[0].gateway = ethernet_settings.gateway;
    TCPIP_HOSTS_CONFIGURATION[0].priDNS = ethernet_settings.pridns;
    TCPIP_HOSTS_CONFIGURATION[0].secondDNS = ethernet_settings.secdns;
    TCPIP_HOSTS_CONFIGURATION[0]. startFlags = ethernet_settings.flags;   
#endif
#ifdef WIFI_ENABLED
#ifdef ETHERNET_ENABLED
    TCPIP_HOSTS_CONFIGURATION[1].hostName = system_settings.nodename;
    TCPIP_HOSTS_CONFIGURATION[1].macAddr = wifi_settings.macaddr;
    TCPIP_HOSTS_CONFIGURATION[1].ipAddr = wifi_settings.ip;
    TCPIP_HOSTS_CONFIGURATION[1].ipMask = wifi_settings.netmask;
    TCPIP_HOSTS_CONFIGURATION[1].gateway = wifi_settings.gateway;
    TCPIP_HOSTS_CONFIGURATION[1].priDNS = wifi_settings.pridns;
    TCPIP_HOSTS_CONFIGURATION[1].secondDNS = wifi_settings.secdns;
    TCPIP_HOSTS_CONFIGURATION[1]. startFlags = wifi_settings.flags;   
#else
    TCPIP_HOSTS_CONFIGURATION[0].hostName = system_settings.nodename;
    TCPIP_HOSTS_CONFIGURATION[0].macAddr = wifi_settings.macaddr;
    TCPIP_HOSTS_CONFIGURATION[0].ipAddr = wifi_settings.ip;
    TCPIP_HOSTS_CONFIGURATION[0].ipMask = wifi_settings.netmask;
    TCPIP_HOSTS_CONFIGURATION[0].gateway = wifi_settings.gateway;
    TCPIP_HOSTS_CONFIGURATION[0].priDNS = wifi_settings.pridns;
    TCPIP_HOSTS_CONFIGURATION[0].secondDNS = wifi_settings.secdns;
    TCPIP_HOSTS_CONFIGURATION[0]. startFlags = wifi_settings.flags;   
#endif
#endif

    
    (void)memset(&tcpipInit, 0, sizeof(tcpipInit));
    tcpipInit.pNetConf = TCPIP_HOSTS_CONFIGURATION;
    tcpipInit.nNets = TCPIP_HOSTS_CONFIGURATION_SIZE;
    tcpipInit.pModConfig = TCPIP_STACK_MODULE_CONFIG_TBL;
    tcpipInit.nModules = TCPIP_STACK_MODULE_CONFIG_TBL_SIZE;
    tcpipInit.initCback = NULL; 

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

#endif	

