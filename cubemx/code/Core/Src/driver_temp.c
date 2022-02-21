/*
 * driver_temp.c
 *
 *  Created on: 28.01.2022.
 *      Author: Test
 */
#include "driver_temp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "adc.h"

static QueueHandle_t TEMP_MailboxHandle;	//freeRTOS mailbox implementation for receiving temperature value
static TaskHandle_t TEMP_TaskHandle;		//freeRTOS thread for handling temperature sensor

//function of the temperature sensor thread
static void TEMP_Task(void* params)
{
	while (1)
	{
		HAL_ADC_Start_IT(&hadc1);					//starting AD converter
		ulTaskNotifyTake(pdFALSE, portMAX_DELAY);

		float value = HAL_ADC_GetValue(&hadc1);		//receiving converted value
		value = value * 5.0 / 4096.0; 				//2^12 = 4096
		value *= 100; 								//value converted to Celsius

		xQueueOverwrite(TEMP_MailboxHandle, &value);//appending the new value to the queue
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

//Callback for ADC interrupt to wakeup the temperature sensor thread
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	if (hadc->Instance == hadc1.Instance)
	{
		BaseType_t woken = pdFALSE;
		vTaskNotifyGiveFromISR(TEMP_TaskHandle, &woken);
		portYIELD_FROM_ISR(woken);
	}
}

//function that returns the last received temperature value
float TEMP_GetCurrentValue()
{
	float result = 0.0;
	xQueuePeek(TEMP_MailboxHandle, &result, portMAX_DELAY);
	return result;
}

//initialization of the thread and the mailbox
void TEMP_Init()
{
	TEMP_MailboxHandle = xQueueCreate(1, sizeof(float));
	xTaskCreate(TEMP_Task, "TEMP_Task", 128, NULL, 10, &TEMP_TaskHandle);
}


