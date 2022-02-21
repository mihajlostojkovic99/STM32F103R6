/*
 * example.c
 *
 *  Created on: 28.01.2022.
 *      Author: Test
 */
#include "example.h"
#include "tim.h"
#include "driver_motor.h"
#include "driver_console.h"
#include "driver_lcd.h"
#include "driver_temp.h"


static void ExampleTask(void* params)
{
	float temp = 0.0;
	while (1)
	{
		temp = TEMP_GetCurrentValue();

		if (temp <= 30.0) MOTOR_Off();
		else if (temp > 30.0 && temp <= 35.0) MOTOR_HalfSpeed();
		else if (temp > 35.0) MOTOR_FullSpeed();

		LCD_ShowTemperature(temp);
		UART_AsyncTransmitTemperature(temp);
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

void exampleInit()
{
	MOTOR_Init();
	UART_Init();
	LCD_Init();
	TEMP_Init();

	xTaskCreate(ExampleTask, "ExampleTask", 64, NULL, 2, NULL);

}
