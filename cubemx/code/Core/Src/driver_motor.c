/*
 * driver_motor.c
 *
 *  Created on: 28.01.2022.
 *      Author: Test
 */
#include "tim.h"

//quick functions for adjusting motor speed

void MOTOR_FullSpeed()
{
	htim3.Instance->CCR1 = 10;
}

void MOTOR_HalfSpeed()
{
	htim3.Instance->CCR1 = 5;
}

void MOTOR_Off()
{
	htim3.Instance->CCR1 = 0;
}

void MOTOR_Init()
{
	htim3.Instance->CCR1 = 0;
	HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);
}
