/*
 * uart.h
 *
 *  Created on: 20.6.2012
 *      Author: ohejda
 *
 *  Description: uart module using rx/tx interrupts.
 *  Functions:
 *  	uart_init .. initialization
 *  	uart_putc .. put char function
 *  	uart_puts .. put string function
 */

#ifndef UART_H_
#define UART_H_

#include <inttypes.h>

void set_adc_val(uint8_t chnl, uint16_t val);

void uart_init(void); // initialization
int uart_putc(char c); // put char function
int uart_puts(char *s); // put string function

#endif /* UART_H_ */
