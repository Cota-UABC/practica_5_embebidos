#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_slave.h"
#include <string.h>
#include "driver/gpio.h"

static const char *TAG = "SPI";

#define MOSI 13
#define MISO 12
#define SPI_SCLK 14
#define SPI_CS 27

#define GPIO_LED_1 25
#define GPIO_LED_2 26

//global
spi_slave_transaction_t spi_slave_transaction;

esp_err_t init_spi_slave();
uint8_t spi_receive();
esp_err_t spi_transmit(uint8_t data);
esp_err_t gpio_init();

esp_err_t init_spi_slave()
{
	spi_bus_config_t spi_bus_config = {
			.mosi_io_num=MOSI,
			.miso_io_num=MISO,
			.sclk_io_num=SPI_SCLK,
			.quadwp_io_num = -1,
			.quadhd_io_num = -1,
	};

	spi_slave_interface_config_t spi_slave_interface_config={
	        .mode=0,
	        .spics_io_num=SPI_CS,
	        .queue_size=3,
	        .flags=0,
	    };

	spi_slave_initialize(SPI2_HOST, &spi_bus_config, &spi_slave_interface_config, SPI_DMA_CH_AUTO);

	return ESP_OK;
}

uint8_t spi_receive()
{
	uint8_t buff[1] = {0};

	memset(&spi_slave_transaction, 0, sizeof(spi_slave_transaction));//valores de 0

	spi_slave_transaction.length=  sizeof(buff) * 8;
	spi_slave_transaction.rx_buffer= buff;

	spi_slave_transmit(SPI2_HOST, &spi_slave_transaction, pdMS_TO_TICKS(10000));

	ESP_LOGI(TAG, "Data received: %d", buff[0]);

	return  buff[0];
}

esp_err_t spi_transmit(uint8_t data)
{
	uint8_t buff[1] = {data};

	memset(&spi_slave_transaction, 0, sizeof(spi_slave_transaction));//valores de 0

	spi_slave_transaction.length=  sizeof(buff) * 8;
	spi_slave_transaction.trans_len=  sizeof(buff) * 8;
	spi_slave_transaction.tx_buffer= buff;
	spi_slave_transmit(SPI2_HOST, &spi_slave_transaction, pdMS_TO_TICKS(10000));

	ESP_LOGI(TAG, "Data transmited: %d", buff[0]);

	return ESP_OK;
}

esp_err_t gpio_init()
{
	gpio_set_direction(GPIO_LED_1, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_LED_2, GPIO_MODE_OUTPUT);

	gpio_set_level(GPIO_LED_1, 0);
	gpio_set_level(GPIO_LED_2, 0);

	return ESP_OK;
}

void app_main(void)
{
	uint8_t received_data = 0, old_received_data = 0;

	ESP_ERROR_CHECK(init_spi_slave());
	ESP_ERROR_CHECK(gpio_init());

	while(true)
	{
		ESP_LOGI(TAG, "------------------");
		old_received_data = received_data;
		received_data = spi_receive();

		//(received_data == 0) ? gpio_set_level(GPIO_LED_1, 0), gpio_set_level(GPIO_LED_2, 0) : 0;
		(received_data == 1) ? gpio_set_level(GPIO_LED_1, 1), gpio_set_level(GPIO_LED_2, 0) : 0;
		(received_data == 2) ? gpio_set_level(GPIO_LED_2, 1), gpio_set_level(GPIO_LED_1, 0) : 0;
		(received_data == 5) ? spi_transmit(old_received_data) : 0;
	}
}
