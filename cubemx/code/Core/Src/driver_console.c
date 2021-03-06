/*
 * driver_console.c
 *
 *  Created on: 28.01.2022.
 *      Author: Test
 */
#include "driver_console.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "usart.h"

static TaskHandle_t TransmitTaskHandle; 		//freeRTOS thread handle for the console
static QueueHandle_t UART_TransmitQueueHandle;	//freeRTOS queue handle for queue of commands to send to the console

//freeRTOS thread function for the console
static void UART_TransmitTask(void* params)
{
	uint8_t buffer;
	while(1)
	{
		xQueueReceive(UART_TransmitQueueHandle, &buffer, portMAX_DELAY);//receiving new value
		HAL_UART_Transmit_IT(&huart1, &buffer, sizeof(uint8_t));		//transmitting value to the console
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);						//waiting for confirmation
	}
}

//Callback function for UART interrupt - new value received
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == huart1.Instance)
	{
		BaseType_t woken = pdFALSE;
		vTaskNotifyGiveFromISR(TransmitTaskHandle, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

//function for sending a command which will display a temperature value from the formal parameter
void UART_AsyncTransmitTemperature(uint32_t temp)
{
	//cleaning the console
	char clean = '\b';
	for (uint32_t i = 0; i < 16; i++)
	{
		xQueueSendToBack(UART_TransmitQueueHandle, &clean, portMAX_DELAY);
	}

	//showing a new temperature value
	char message[13] = "Temperatura: ";
	for (uint32_t i = 0; i < 13; i++)
	{
		xQueueSendToBack(UART_TransmitQueueHandle, message + i, portMAX_DELAY);
	}

	char digits[3];
	uint32_t index = 3;
	while (index >= 0 && temp != 0)
	{
		digits[--index] = '0' + temp % 10;
		temp /= 10;
	}

	for (uint32_t i = index; i < 3; i++)
	{
			xQueueSendToBack(UART_TransmitQueueHandle, digits + i, portMAX_DELAY);
	}


}

//initializing the thread and the queue
void UART_Init()
{
	xTaskCreate(UART_TransmitTask, "UART_TransmitTask", 64, NULL, 5, &TransmitTaskHandle);
	UART_TransmitQueueHandle = xQueueCreate(64, sizeof(uint8_t));
}
