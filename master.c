#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static const char *TAG = "SPI";
static const char *TAG_B = "Boton";


#define GPIO_MOSI 13
#define GPIO_MISO 12
#define GPIO_SCLK 14
#define GPIO_CS 27

#define STACK_SIZE 1024*2

#define GPIO_BUTTON_1 15
#define GPIO_BUTTON_2 16

//global
spi_device_handle_t handle;
spi_transaction_t spi_transaction;

uint8_t value = 0;

esp_err_t init_spi_master();
esp_err_t spi_write(uint8_t value);
esp_err_t gpio_init();
void vTaskBotones( void * pvParameters );

esp_err_t init_spi_master()
{
	// Configuration for the SPI bus
	spi_bus_config_t buscfg = {
		.mosi_io_num = GPIO_MOSI,
		.miso_io_num = GPIO_MISO,
		.sclk_io_num = GPIO_SCLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1};

	// Configuration for the SPI device on the other side of the bus
	spi_device_interface_config_t devcfg = {
		.command_bits = 0,
		.address_bits = 0,
		.dummy_bits = 0,
		.clock_speed_hz = 5000000,
		.duty_cycle_pos = 128, // 50% duty cycle
		.mode = 0,
		.spics_io_num = GPIO_CS,
		.cs_ena_posttrans = 3, // Keep the CS low 3 cycles after transaction
		.queue_size = 3};


    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &devcfg, &handle);

	return ESP_OK;
}

esp_err_t spi_write(uint8_t value)
{
	uint8_t sendbuf[1] = {0};

	memset(&spi_transaction, 0, sizeof(spi_transaction));//clear struct

	sendbuf[0] = value;

	spi_transaction.length = sizeof(sendbuf) * 8;
	spi_transaction.tx_buffer = sendbuf;
	spi_device_transmit(handle, &spi_transaction);

	ESP_LOGI(TAG, "Data send: %d", sendbuf[0]);

	return ESP_OK;
}

esp_err_t gpio_init()
{
	gpio_set_direction(GPIO_BUTTON_1, GPIO_MODE_INPUT);
	gpio_set_direction(GPIO_BUTTON_2, GPIO_MODE_INPUT);

	gpio_pullup_en(GPIO_BUTTON_1);
	gpio_pullup_en(GPIO_BUTTON_2);

	return ESP_OK;
}

void vTaskBotones( void * pvParameters )
{

	while(true)
	{
		if( gpio_get_level(GPIO_BUTTON_1) == 0)
		{
			//ESP_LOGI(TAG_B, "Button 1 pressed");
			value = 1;
		}
		else if(gpio_get_level(GPIO_BUTTON_2) == 0)
		{
			//ESP_LOGI(TAG_B, "Button 2 pressed");
			value = 2;
		}
		//else value = 0;

		//ESP_LOGI(TAG_B, "Button task. HI!!!");

		vTaskDelay(pdMS_TO_TICKS(10));//10 mili second
	}
}

void app_main(void)
{
	ESP_ERROR_CHECK(init_spi_master());
	ESP_ERROR_CHECK(gpio_init());

	static uint8_t ucParameterToPass;
	TaskHandle_t xHandle = NULL;

	xTaskCreate(
		 vTaskBotones,
		 "vTaskBotones",
		 STACK_SIZE,
		 &ucParameterToPass,
		 1,
		 &xHandle );

    while (1)
    {
    	ESP_ERROR_CHECK(spi_write(value));

    	vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
