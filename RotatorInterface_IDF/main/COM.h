/*
 * COM.h
 * For ESP-IDF
 *  Created on: Mar 13, 2020
 *      Author: Zening
 */

#ifndef COM_H_
#define COM_H_

#include "kfifo.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"

// #define COM_ENABLE_TXE(huart) ((huart).Instance->CR1 |= USART_CR1_TXEIE)
// #define COM_DISABLE_TXE(huart) ((huart).Instance->CR1 &= ~(USART_CR1_TXEIE))

#define min(x,y) ({ \
    typeof(x) _x = (x);    \
    typeof(y) _y = (y);    \
    (void) (&_x == &_y);    \
    _x < _y ? _x : _y; })



typedef struct {
	KFIFO FIFO_TX;
	KFIFO FIFO_RX;
	uart_port_t uart_num;
    uart_config_t uart_config;
} COM_t;

extern COM_t COM1;

#define COM_BUFFER_LEN 128

extern QueueHandle_t pCOMEventQueue; // queue for interrupt events

// lenbuf must be round power of 2
void COM_init(COM_t *p,  uart_port_t uart_num, int tx_pin, int rx_pin,
			uint32_t baud_rate, size_t lenbufTX, size_t lenbufRX);


size_t COM_send_message(COM_t* p,  const uint8_t* s, size_t n);

size_t COM_read_message(COM_t* p, uint8_t* buf, size_t n);


#endif /* COM_H_ */
