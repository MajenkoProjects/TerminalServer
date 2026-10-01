#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "driver/spi_slave.h"
#include "driver/gpio.h"


#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_private/wifi.h"
#include "esp_netif_net_stack.h"

#include "wifi_bridge.h"

#define GPIO_MOSI           10
#define GPIO_MISO           14
#define GPIO_SCLK           12
#define GPIO_CS             11
#define GPIO_INT            13
#define GPIO_LED1           16
#define GPIO_LED2           17
#define GPIO_LED3           18
#define GPIO_LED4           8



#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

struct mac {
    bool used;
    uint8_t address[6];
};

#define NUM_MACS 32

uint32_t activity_rx_led_ticks = 0;
uint32_t activity_tx_led_ticks = 0;

struct mac macs[NUM_MACS] = { 
    { // Broadcast
        .used = true,
        .address = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
    }
};


static const spi_bus_config_t buscfg = {
    .mosi_io_num = GPIO_MOSI,
    .miso_io_num = GPIO_MISO,
    .sclk_io_num = GPIO_SCLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
};

enum state {
	STATE_INIT = 0,
	STATE_INIT_SPI,
	STATE_INIT_WIFI,
	STATE_QUEUE_XFER,
	STATE_WAIT_XFER,
	STATE_GOT_XFER,
};

wifi_status_t wifi_status = {
    .state = WIFI_IDLE
};

enum state state = STATE_INIT;
bool first_mac_set = false;

QueueHandle_t wifi_to_spi_queue;

void init_io() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_INT),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << GPIO_LED1);
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << GPIO_LED2);
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << GPIO_LED3);
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << GPIO_LED4);
    gpio_config(&io_conf);

    gpio_set_level(GPIO_INT, 1);
    gpio_set_level(GPIO_LED1, 0);
    gpio_set_level(GPIO_LED2, 0);
    gpio_set_level(GPIO_LED3, 0);
    gpio_set_level(GPIO_LED4, 1);
}


void my_post_setup_cb(spi_slave_transaction_t *trans)
{
}

void my_post_trans_cb(spi_slave_transaction_t *trans)
{
}

 
static const spi_slave_interface_config_t slvcfg = {
    .mode = 0,
    .spics_io_num = GPIO_CS,
    .queue_size = 1,
    .flags = 0,
    .post_setup_cb = my_post_setup_cb,
    .post_trans_cb = my_post_trans_cb
};


//static EventGroupHandle_t s_wifi_event_group;


// Callback signature required by ESP-IDF Wi-Fi driver
static esp_err_t wifi_l2_rx_cb(void *buffer, uint16_t len, void *eb)
{
#if 0
    //uint8_t *buf = (uint8_t *)buffer;

    //for (int i = 0; i < len; i++) {
    //    if ((i%16)==0) printf("\r\n");
        //printf("%02x ", buf[i]);
    //}
    //printf("\r\n");

#endif
    bool allowed = false;
    for (int i = 0; i < NUM_MACS; i++) {
        if (macs[i].used == false) continue;
        if (memcmp(macs[i].address, buffer, 6) == 0) {
            allowed = true;
            break;
        }
    }

    if (!allowed) {
        //printf("Packet rejected\r\n");
        esp_wifi_internal_free_rx_buffer(eb);
        return ESP_OK;        
    }
    
    
    gpio_set_level(GPIO_LED2, 1);
    activity_rx_led_ticks = xTaskGetTickCount();
    
    
    //printf("Got RX packet length %d\r\n", len);
    if (buffer && len > 0) {
        gpio_set_level(GPIO_INT, 0);
        l2_packet_t pkt;
        pkt.length = len;
        pkt.payload = malloc(len);
        memcpy(pkt.payload, buffer, len);

        if (xQueueSendToBack(wifi_to_spi_queue, &pkt, 0) != pdTRUE) {
            free(pkt.payload); // Queue full, drop packet
        }
    }

    // CRITICAL: You MUST free the internal Wi-Fi RX buffer back to the driver
    esp_wifi_internal_free_rx_buffer(eb);
    return ESP_OK;
}

static void event_handler(void* arg, esp_event_base_t event_base,
        int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        gpio_set_level(GPIO_LED1, 0);
        //printf("WiFi disconnected.\r\n");
        wifi_status.state = WIFI_DISCONNECTED;
        wifi_status.link = 0;
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        gpio_set_level(GPIO_LED1, 1);
        //printf("WiFi connected.\r\n");
        ESP_ERROR_CHECK(esp_wifi_internal_reg_rxcb(WIFI_IF_STA, wifi_l2_rx_cb));
        wifi_status.link = 1;
        wifi_status.state = WIFI_CONNECTED;
    }
}

#define RCV_HOST SPI2_HOST

esp_err_t send_raw_wifi_frame(void *netif, void *buffer, size_t len)
{
    return esp_wifi_internal_tx(WIFI_IF_STA, (void *)buffer, len);
}

void wifi_init_sta(void)
{
// 1. Initialize TCP/IP netif stack FIRST
    ESP_ERROR_CHECK(esp_netif_init());

    // 2. Create default system event loop required by esp_netif Wi-Fi handlers
    esp_err_t err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_ERROR_CHECK(err); // Ignore if default loop was already created elsewhere
    }

    // 3. Create default STA netif instance
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    
    esp_netif_dhcpc_stop(sta_netif);

    // 4. Initialize Wi-Fi driver with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = 0;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));


    
    
    
    // Register L2 hook AFTER starting Wi-Fi
/*
    s_wifi_event_group = xEventGroupCreate();


    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = 0;

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
*/
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    /*
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
*/
    
    
        // 5. Set mode to STA and start Wi-Fi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 6. Register L2 RX callback AFTER esp_wifi_start()
    ESP_ERROR_CHECK(esp_wifi_internal_reg_rxcb(WIFI_IF_STA, wifi_l2_rx_cb));
	return;
}


void app_main(void)
{
	char ssid[33] = {0};
	char psk[65] = {0};
	int bytes = 0;
	uint8_t *tx = NULL;
	uint8_t *rx = NULL;

    init_io();
	spi_slave_transaction_t transaction = { 0 };
	transaction.length = SPI_MAX_SIZE * 8;

	spi_slave_transaction_t *result = NULL;
    wifi_to_spi_queue = xQueueCreate(32, sizeof(l2_packet_t));
    l2_packet_t pkt;
    
    int tmpint;
    
    while (1) {

        
        if (activity_rx_led_ticks > 0) {
            if ((xTaskGetTickCount() - activity_rx_led_ticks) > 10) {
                gpio_set_level(GPIO_LED2, 0);
                activity_rx_led_ticks = 0;
            }
        }

        if (activity_tx_led_ticks > 0) {
            if ((xTaskGetTickCount() - activity_tx_led_ticks) > 10) {
                gpio_set_level(GPIO_LED3, 0);
                activity_tx_led_ticks = 0;
            }
        }


        if (uxQueueMessagesWaiting(wifi_to_spi_queue) == 0) {
            gpio_set_level(GPIO_INT, 1);
        }

        
		switch (state) {
			case STATE_INIT:
				//printf("ESP32 WiFi MAC Bridge (c) 2026 Majenko Technologies\r\n");
				state = STATE_INIT_SPI;
				break;

			case STATE_INIT_SPI:
				gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
				gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
				gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);
				spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
				transaction.tx_buffer = tx = spi_bus_dma_memory_alloc(RCV_HOST, SPI_MAX_SIZE, 0);
				transaction.rx_buffer = rx = spi_bus_dma_memory_alloc(RCV_HOST, SPI_MAX_SIZE, 0);
				//printf("SPI interface configured\r\n");
				state = STATE_INIT_WIFI;
				break;

			case STATE_INIT_WIFI:
				wifi_init_sta();
                //printf("WiFI interface configured\r\n");
				state = STATE_QUEUE_XFER;
				break;

			case STATE_QUEUE_XFER:
            	transaction.length = SPI_MAX_SIZE * 8;
                transaction.tx_buffer = tx;
                transaction.rx_buffer = rx;
				if (spi_slave_queue_trans(RCV_HOST, &transaction, 1) == ESP_OK) {
					//printf("Waiting for transaction...\r\n");
					state = STATE_WAIT_XFER;
				}
				break;

			case STATE_WAIT_XFER:
				if (spi_slave_get_trans_result(RCV_HOST, &result, 1) == ESP_OK) {
					bytes = result->trans_len / 8;
					//rintf("Got transaction 0x%02x of %d bytes.\r\n", rx[0], bytes);
					state = STATE_GOT_XFER;
				}
				break;

			case STATE_GOT_XFER:
				switch (rx[0]) {
                    case REG_RESET:
                        memset(ssid, 0, 33);
                        memset(psk, 0, 65);
                        xQueueReset(wifi_to_spi_queue);
                        esp_wifi_stop();
                        wifi_status.state = WIFI_IDLE;
                        break;
                        
					case REG_SSID:
						if (bytes > 1) {
							memset(ssid, 0, 33);
							memcpy(ssid, &rx[1], bytes - 1);
                            //printf("SSID: %s\r\n", ssid);
						}
						break;
					case REG_PSK:
						if (bytes > 1) {
							memset(psk, 0, 65);
							memcpy(psk, &rx[1], bytes - 1);
                            //printf("PSK: %s\r\n", psk);
						}
						break;
					case REG_STATUS:
                        esp_wifi_sta_get_rssi(&tmpint);
                        wifi_status.rssi = htonl(tmpint);
                        wifi_status.pending = uxQueueMessagesWaiting(wifi_to_spi_queue);
                        if (wifi_status.pending > 0) {
                            xQueuePeek(wifi_to_spi_queue, &pkt, 1);
                            wifi_status.nextsize = htons(pkt.length);
                        } else {
                            wifi_status.nextsize = 0;
                        }
                        memcpy(tx, wifi_status.val, sizeof(wifi_status_t));
						break;
					case REG_RBSTAT:
                        tx[0] = uxQueueMessagesWaiting(wifi_to_spi_queue);
                        if (tx[0] > 0) {
                            xQueuePeek(wifi_to_spi_queue, &pkt, 1);
                            tx[1] = pkt.length >> 8;
                            tx[2] = pkt.length;
                            //printf("%d in queue, next is %d\r\n", tx[0], pkt.length);
                        } else {
                            tx[1] = 0;
                            tx[2] = 0;
                        }
						break;
					case REG_RPACKET:
                        if (xQueueReceive(wifi_to_spi_queue, &pkt, 1) == pdTRUE) {
                            //printf("Send packet %d\r\n", pkt.length);
                            tx[0] = pkt.length >> 8;
                            tx[1] = pkt.length;
                            memcpy(&tx[2], pkt.payload, pkt.length);
                            free(pkt.payload);
                        }
						break;
					case REG_WPACKET:
                        //printf("TX L2 Frame Len=%d | Dst: %02x:%02x:%02x:%02x:%02x:%02x | Src: %02x:%02x:%02x:%02x:%02x:%02x | Type: 0x%02x%02x\n",
                        //        bytes - 1,
                        //        rx[1], rx[2], rx[3], rx[4], rx[5], rx[6],
                        //        rx[7], rx[8], rx[9], rx[10], rx[11], rx[12],
                        //        rx[13], rx[14]);

                        ESP_ERROR_CHECK(esp_wifi_internal_tx(WIFI_IF_STA, (void *)&rx[1], bytes - 1));
                        gpio_set_level(GPIO_LED3, 1);
                        activity_tx_led_ticks = xTaskGetTickCount();
                       // vTaskDelay(pdMS_TO_TICKS(10));
                        break;
					case REG_ADDMAC:
                        if (bytes == 7) {
                            if (!first_mac_set) {
                                esp_wifi_set_mac(WIFI_IF_STA, &rx[1]);
                                first_mac_set = true;
                            }
                            for (int i = 0; i < NUM_MACS; i++) {
                                if (macs[i].used == false) {
                                    macs[i].address[0] = rx[1];
                                    macs[i].address[1] = rx[2];
                                    macs[i].address[2] = rx[3];
                                    macs[i].address[3] = rx[4];
                                    macs[i].address[4] = rx[5];
                                    macs[i].address[5] = rx[6];
                                    macs[i].used = true;
                                    break;
                                }
                            }
                        }
						break;
					case REG_DELMAC:
						break;
				}
				state = STATE_QUEUE_XFER;
				break;
		}
        
        switch (wifi_status.state) {
            case WIFI_IDLE:
                if (strlen(ssid) > 0) {
                    //printf("Attempting connection to WiFi...\r\n");
                    wifi_status.state = WIFI_CONNECT;
                }
                break;
                
            case WIFI_CONNECT:
                wifi_config_t wifi_config = { 0 };
                esp_wifi_stop();
                strcpy((char *)wifi_config.sta.ssid, ssid);
                strcpy((char *)wifi_config.sta.password, psk);	
                esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
                esp_wifi_start();
                esp_wifi_connect();
                wifi_status.state = WIFI_CONNECTING;
                break;
                
            case WIFI_CONNECTING:
                break;
                
            case WIFI_CONNECTED:
                // This is where we read and write packets
                break;

            case WIFI_DISCONNECTED:
                //printf("Retrying the connection\r\n");
                wifi_status.state = WIFI_CONNECT;
                break;
        }
        
        
        
    }
}
