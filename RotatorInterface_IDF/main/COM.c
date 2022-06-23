/*
 * COM.c
 *
 *  Created on: Mar 13, 2020
 *      Author: Zening
 */


#include "COM.h"
#include "string.h"
#include "stdlib.h"


QueueHandle_t pCOMEventQueue; // queue for interrupt events
void COM_init(COM_t *p,  uart_port_t uart_num, int tx_pin, int rx_pin,
			uint32_t baud_rate, size_t lenbufTX, size_t lenbufRX)
{
	// kfifo_static_init(&(p->FIFO_TX), bufTX, lenbufTX);
	// kfifo_static_init(&(p->FIFO_RX), bufRX, lenbufRX);
	uart_config_t uart_config = {
		.baud_rate = baud_rate,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	};
	p->uart_num = uart_num;
	p->uart_config = uart_config;
	ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
	uart_set_pin(uart_num, tx_pin, rx_pin, 0, 0);
	pCOMEventQueue = xQueueCreate(16, sizeof(void*));
	uart_driver_install(uart_num, lenbufRX, lenbufTX, 16, &pCOMEventQueue, ESP_INTR_FLAG_IRAM );
}




size_t COM_send_message(COM_t* p,  const uint8_t* s, size_t n)
{
	return uart_write_bytes(p->uart_num,  s, n);
}

size_t COM_read_message(COM_t* p, uint8_t* buf, size_t n)
{
	// uart_read_bytes(p->uart_num, buf, n, );
	return 0;
}