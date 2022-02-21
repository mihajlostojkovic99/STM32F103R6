/*
 * driver_lcd.c
 *
 *  Created on: 28.01.2022.
 *      Author: Test
 */
#include "driver_lcd.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#define LCD_ENABLE_BIT 0x40 //Enable bit for sending values to the LCD

static TaskHandle_t LCD_TaskHandle;	//freeRTOS thread handle for LCD
QueueHandle_t LCD_QueueHandle;		//freeRTOS queue handle for queue of commands to send to the screen

static void LCD_Write(LCD_CommandReg reg, LCD_CommandVal val)
{
	GPIOC->ODR = ((reg & 0x01) << 5) | (val & 0x0F); 	//Sending new value to the LCD
	GPIOC->ODR |= LCD_ENABLE_BIT;						//Setting the enable bit
	GPIOC->ODR &= ~LCD_ENABLE_BIT;						//Resetting the enable bit
}

static void LCD_CommandInit(LCD_CommandReg reg, LCD_CommandVal val)
{
	LCD_Write(reg, val >> 4);		//Writing higher 4 bits to the LCD
	LCD_Write(reg, val);			//Writing lower 4 bits to the LCD
	vTaskDelay(pdMS_TO_TICKS(2));	//Delay for the LCD to refresh
}

//freeRTOS thread function for the LCD screen
static void LCD_Task(void* params)
{
	vTaskDelay(pdMS_TO_TICKS(20));
	LCD_Write(LCD_INSTRUCTION, (LCD_FUNCTION_SET_4_BIT_INTERFACE | LCD_FUNCTION_SET_INSTRUCTION) >> 4);
	vTaskDelay(pdMS_TO_TICKS(2));
	LCD_CommandInit(LCD_INSTRUCTION, LCD_FUNCTION_SET_INSTRUCTION | LCD_FUNCTION_SET_4_BIT_INTERFACE | LCD_FUNCTION_SET_2_LINE | LCD_FUNCTION_SET_5x8_DOTS);
	LCD_CommandInit(LCD_INSTRUCTION, LCD_CONTROL_INSTRUCTION | LCD_CONTROL_DISPLAY_ON);
	LCD_CommandInit(LCD_INSTRUCTION, LCD_ENTRY_MODE_INSTRUCTION | LCD_ENTRY_MODE_INC_ADR | LCD_ENTRY_MODE_SHIFT_OFF);
	LCD_CommandInit(LCD_INSTRUCTION, LCD_RETURN_HOME_INSTRUCTION);

	LCD_Command cmd;
	while (1)
	{
		xQueueReceive(LCD_QueueHandle, &cmd, portMAX_DELAY);	//receive a new command from queue
		LCD_CommandInit(cmd.reg, cmd.val);						//send the command to the LCD screen
	}
}

//function for sending a new command to the screen
void LCD_CommandEnqueue(LCD_CommandReg reg, LCD_CommandVal val)
{
	LCD_Command cmd;
	cmd.reg = reg;
	cmd.val = val;
	xQueueSend(LCD_QueueHandle, &cmd, portMAX_DELAY);	//append a new command for the LCD screen
}

//function for sending a new command to the screen from a interrupt
void LCD_CommandEnqueueFromISR(LCD_CommandReg reg, LCD_CommandVal val, BaseType_t* pxHigherPriorityTaskWoken)
{
	LCD_Command cmd;
	cmd.reg = reg;
	cmd.val = val;
	xQueueSendFromISR(LCD_QueueHandle, &cmd, pxHigherPriorityTaskWoken);
}

//function for sending a command which will display a temperature value from the formal parameter
void LCD_ShowTemperature(uint32_t temp)
{
	//cleaning the display
	LCD_CommandEnqueue(LCD_INSTRUCTION, LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x0D);
	char clean = ' ';
	for (uint32_t i = 0; i < 3; i++)
	{
		LCD_CommandEnqueue(LCD_DATA, clean);
	}

	//showing a new temperature value
	LCD_CommandEnqueue(LCD_INSTRUCTION, LCD_SET_DD_RAM_ADDRESS_INSTRUCTION | 0x00);
	char str[13] = "Temperatura: ";
	for (uint32_t i = 0; i < 13; i++)
	{
		LCD_CommandEnqueue(LCD_DATA, str[i]);
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
		LCD_CommandEnqueue(LCD_DATA, digits[i]);
	}

}

//initializing the thread and the queue
void LCD_Init()
{
	LCD_QueueHandle = xQueueCreate(64, sizeof(LCD_Command));
	xTaskCreate(LCD_Task, "LCD_Task", 64, NULL, 5, &LCD_TaskHandle);
}
