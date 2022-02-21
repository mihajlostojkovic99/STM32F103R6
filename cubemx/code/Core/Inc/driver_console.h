/*
 * driver_console.h
 *
 *  Created on: 28.01.2022.
 *      Author: Test
 */

#ifndef CORE_INC_DRIVER_CONSOLE_H_
#define CORE_INC_DRIVER_CONSOLE_H_
#include <stdint.h>

extern void UART_AsyncTransmitTemperature(uint32_t temp);
extern void UART_Init();

#endif /* CORE_INC_DRIVER_CONSOLE_H_ */
