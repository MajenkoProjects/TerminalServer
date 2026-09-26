#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/spi_slave.h"
#include "driver/gpio.h"
#include "esp_wifi.h"



#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"



#define SPI_MAX_SIZE 1523
#define GPIO_MOSI           10
#define GPIO_MISO           14
#define GPIO_SCLK           12
#define GPIO_CS             11



#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = GPIO_MISO,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
};


void my_post_setup_cb(spi_slave_transaction_t *trans)
{
}

void my_post_trans_cb(spi_slave_transaction_t *trans)
{
}

 
static const spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = GPIO_CS,
        .queue_size = 3,
        .flags = 0,
        .post_setup_cb = my_post_setup_cb,
        .post_trans_cb = my_post_trans_cb
};


static const char *TAG = "wifi station";

static int s_retry_num = 0;

static EventGroupHandle_t s_wifi_event_group;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 10) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}






#define RCV_HOST SPI2_HOST


enum registers {
	REG_SSID = 0x80,
	REG_PSK,
	REG_STATUS,
	REG_SIGNAL,
	REG_PBSTAT,
	REG_RPACKET,
	REG_WPACKET,
	REG_ADDMAC,
	REG_DELMAC
};

enum state {
	STATE_INIT = 0,
	STATE_INIT_SPI,
	STATE_INIT_WIFI,
	STATE_QUEUE_XFER,
	STATE_WAIT_XFER,
	STATE_GOT_XFER,
};

enum state state = STATE_INIT;




void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    cfg.nvs_enable = 0;

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

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

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

	return;
}




//Main application
void app_main(void)
{
	char ssid[33];
	char psk[65];
	int bytes = 0;
	const uint8_t *tx = NULL;
	const uint8_t *rx = NULL;

	spi_slave_transaction_t transaction = { 0 };
	transaction.length = SPI_MAX_SIZE * 8;

	spi_slave_transaction_t *result = NULL;

	while (1) {

		switch (state) {
			case STATE_INIT:
				printf("ESP32 WiFi MAC Bridge (c) 2026 Majenko Technologies\r\n");
				state = STATE_INIT_SPI;
				break;

			case STATE_INIT_SPI:
				gpio_set_pull_mode(GPIO_MOSI, GPIO_PULLUP_ONLY);
				gpio_set_pull_mode(GPIO_SCLK, GPIO_PULLUP_ONLY);
				gpio_set_pull_mode(GPIO_CS, GPIO_PULLUP_ONLY);
				spi_slave_initialize(RCV_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
				transaction.tx_buffer = spi_bus_dma_memory_alloc(RCV_HOST, SPI_MAX_SIZE, 0);
				transaction.rx_buffer = spi_bus_dma_memory_alloc(RCV_HOST, SPI_MAX_SIZE, 0);
				printf("SPI interface configured\r\n");
				state = STATE_INIT_WIFI;
				break;

			case STATE_INIT_WIFI:
				wifi_init_sta();
				printf("WiFI interface configured\r\n");
				state = STATE_QUEUE_XFER;
				break;

			case STATE_QUEUE_XFER:
				if (spi_slave_queue_trans(RCV_HOST, &transaction, 1) == ESP_OK) {
					printf("Waiting for transaction...\r\n");
					state = STATE_WAIT_XFER;
				}
				break;

			case STATE_WAIT_XFER:
				if (spi_slave_get_trans_result(RCV_HOST, &result, 1) == ESP_OK) {
					printf("Got transaction.\r\n");
					rx = result->rx_buffer;
					tx = result->tx_buffer;
					bytes = result->trans_len / 8;
					state = STATE_GOT_XFER;
				}
				break;

			case STATE_GOT_XFER:
				printf("Got %d bytes with code %d\r\n", bytes, rx[0]);
				switch (rx[0]) {
					case REG_SSID:
						if (bytes > 1) {
							memset(ssid, 0, 33);
							memcpy(ssid, &rx[1], bytes - 1);
							printf("SSID set to [%s]\r\n", ssid);
						}
						break;
					case REG_PSK:
						if (bytes > 1) {
							memset(psk, 0, 65);
							memcpy(psk, &rx[1], bytes - 1);
							printf("PSK set to [%s]\r\n", psk);
							wifi_config_t wifi_config = { 0 };

							ESP_ERROR_CHECK(esp_wifi_stop() );
							strcpy((char *)wifi_config.sta.ssid, ssid);
							strcpy((char *)wifi_config.sta.password, psk);	
							ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
							ESP_ERROR_CHECK(esp_wifi_start() );
							esp_wifi_connect();
						}
						break;
					case REG_STATUS:
						break;
					case REG_SIGNAL:
						break;
					case REG_PBSTAT:
						break;
					case REG_RPACKET:
						break;
					case REG_WPACKET:
						break;
					case REG_ADDMAC:
						break;
					case REG_DELMAC:
						break;
				}
				state = STATE_QUEUE_XFER;
				break;
		}
    }
}
