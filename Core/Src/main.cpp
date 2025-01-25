#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "tim.h"
#include "i2c.h"
#include "mpu6050.h"

#include <stdio.h>

// Used for debug only - Do not forget to remove props ! :D
#define MOTOR_CAN_TURN 0 	// 0 = No :(
							// 1 = Caution :)

#define MIN_PWM_INPUT_RECEIVER 1000 	// 1ms (5% de 20ms)
#define MAX_PWM_INPUT_RECEIVER 2000 // 2ms (10% de 20ms)
#define MAX_PWM_COUNTER 3600 // 3600 sur 36000 = 10%
#define MIN_PWM_COUNTER 1800 // 1800 sur 36000 = 5%

void SystemClock_Config(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin); // Callback on the GPIO pins
uint32_t constrain(uint32_t min, uint32_t max, uint32_t value);


// PWM From the receiver
volatile uint32_t PWM_Roll = 0;
volatile uint32_t PWM_Pitch = 0;
volatile uint32_t PWM_Thrust = 0;
volatile uint32_t PWM_Yaw = 0;

// PWM output calculated
uint32_t PWM_CHANNEL1_PERIOD = 0; // 1800 - 3600
uint32_t PWM_CHANNEL2_PERIOD = 0; // 1800 - 3600
uint32_t PWM_CHANNEL3_PERIOD = 0; // 1800 - 3600
uint32_t PWM_CHANNEL4_PERIOD = 0; // 1800 - 3600

// Security
bool KILL_SWITCH = false;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init(); 			// GPIO init (normal ones and exti)
  MX_USART1_UART_Init(); 	// usart
  MX_I2C1_Init();			// i2c
  MX_TIM1_Init();			// Timer for the measures of the PWM
  MX_TIM2_Init();			// Timer for the generation of the PWM

  // Starting the timer for the PWM measures
  HAL_TIM_Base_Start(&htim1);
  __HAL_TIM_SET_COUNTER(&htim1,0);

  // PWM Generator for the motors
  if(MOTOR_CAN_TURN){
	  HAL_TIM_Base_Start(&htim2);
	  __HAL_TIM_SET_COUNTER(&htim2,0);
	  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

	  /*
	   * PWM are based on the counter of timer2 ; between 0 to 36'000
	   * If you want 50% duty cycle, you must put the CHANNEL at 18'000
	   * The normal use of the PWM for the motors to turn (with FS-i10AB receiver)
	   * is between 5% and 10% at 50Hz - 1'800 to 3'600
	   */
	  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0); // Gestion du PWM avec le Thrust
	  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0); // Gestion du PWM avec le Thrust
	  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0); // Gestion du PWM avec le Thrust
	  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0); // Gestion du PWM avec le Thrust
  }

  HAL_Delay(3000);

  // UART
  char buffer[64];

  // mpu6050
  MPU6050_t MPU6050;
  while(MPU6050_Init(&hi2c1) == 1);

  while (1)
  {
	  MPU6050_Read_All(&hi2c1, &MPU6050);

	  if(KILL_SWITCH){
		  Error_Handler();
	  }

	  if(MOTOR_CAN_TURN){
		  // testing
		  PWM_CHANNEL3_PERIOD = (PWM_Thrust - 10) * 1800/1000;
		  PWM_CHANNEL3_PERIOD = constrain(MIN_PWM_COUNTER, MAX_PWM_COUNTER, PWM_CHANNEL3_PERIOD);

		  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, PWM_CHANNEL3_PERIOD); // Gestion du PWM avec le Thrust
		  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, PWM_CHANNEL3_PERIOD); // Gestion du PWM avec le Thrust
		  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, PWM_CHANNEL3_PERIOD); // Gestion du PWM avec le Thrust
		  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, PWM_CHANNEL3_PERIOD); // Gestion du PWM avec le Thrust
	  }

	  // DEBUG
	  sprintf(buffer, "%ld, %ld, %ld, %ld, %f, %f\n", PWM_Thrust, PWM_Roll, PWM_Yaw, PWM_Pitch, 1000.f, 2000.f);
	  debug_uart(buffer);
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

// Redefinition of __weak HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// Callbacks
	if(GPIO_Pin == PWM_Killswitch_Pin){ // Killswitch PWM measure
		uint32_t Current_Ticks = __HAL_TIM_GET_COUNTER(&htim1); // Not outside the if condition because time measure
		uint32_t PWM_Time = 0;
		static uint32_t Previous_Killswitch_Ticks = 0;

		if(HAL_GPIO_ReadPin(PWM_Killswitch_GPIO_Port, PWM_Killswitch_Pin)){
			Previous_Killswitch_Ticks = Current_Ticks;
		}
		else{
			if(Current_Ticks > Previous_Killswitch_Ticks){
				PWM_Time = Current_Ticks - Previous_Killswitch_Ticks;
			}
			else{
				PWM_Time = 0xFFFF - Previous_Killswitch_Ticks + Current_Ticks - 1;
			}
		}

		if(PWM_Time > 1250){
			KILL_SWITCH = true;

			PWM_CHANNEL1_PERIOD = 1750;
			PWM_CHANNEL2_PERIOD = 1750;
			PWM_CHANNEL3_PERIOD = 1750;
			PWM_CHANNEL4_PERIOD = 1750;
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 1750);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 1750);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 1750);
			__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 1750);

			HAL_GPIO_TogglePin(LED_13_GPIO_Port, LED_13_Pin);
		}
	}

	else if(GPIO_Pin == PWM_Roll_Pin){
		uint32_t Current_Ticks = __HAL_TIM_GET_COUNTER(&htim1); // Not outside the if condition because time measure
		static uint32_t Previous_Roll_Ticks = 0;

		if(HAL_GPIO_ReadPin(PWM_Roll_GPIO_Port, PWM_Roll_Pin)){
			Previous_Roll_Ticks = Current_Ticks;
		}
		else{
			if(Current_Ticks > Previous_Roll_Ticks){
				PWM_Roll = Current_Ticks - Previous_Roll_Ticks;
			}
			else{
				PWM_Roll = 0xFFFF - Previous_Roll_Ticks + Current_Ticks - 1;
			}
		}
	}

	else if(GPIO_Pin == PWM_Pitch_Pin){
		uint32_t Current_Ticks = __HAL_TIM_GET_COUNTER(&htim1); // Not outside the if condition because time measure
		static uint32_t Previous_Pitch_Ticks = 0;

		if(HAL_GPIO_ReadPin(PWM_Pitch_GPIO_Port, PWM_Pitch_Pin)){
			Previous_Pitch_Ticks = Current_Ticks;
		}
		else{
			if(Current_Ticks > Previous_Pitch_Ticks){
				PWM_Pitch = Current_Ticks - Previous_Pitch_Ticks;
			}
			else{
				PWM_Pitch = 0xFFFF - Previous_Pitch_Ticks + Current_Ticks - 1;
			}
		}
	}

	else if(GPIO_Pin == PWM_Thrust_Pin){
		uint32_t Current_Ticks = __HAL_TIM_GET_COUNTER(&htim1); // Not outside the if condition because time measure
		static uint32_t Previous_Thrust_Ticks = 0;

		if(HAL_GPIO_ReadPin(PWM_Thrust_GPIO_Port, PWM_Thrust_Pin)){
			Previous_Thrust_Ticks = Current_Ticks;
		}
		else{
			if(Current_Ticks > Previous_Thrust_Ticks){
				PWM_Thrust = Current_Ticks - Previous_Thrust_Ticks;
			}
			else{
				PWM_Thrust = 0xFFFF - Previous_Thrust_Ticks + Current_Ticks - 1;
			}
		}
	}

	else if(GPIO_Pin == PWM_Yaw_Pin){
		uint32_t Current_Ticks = __HAL_TIM_GET_COUNTER(&htim1); // Not outside the if condition because time measure
		static uint32_t Previous_Yaw_Ticks = 0;

		if(HAL_GPIO_ReadPin(PWM_Yaw_GPIO_Port, PWM_Yaw_Pin)){
			Previous_Yaw_Ticks = Current_Ticks;
		}
		else{
			if(Current_Ticks > Previous_Yaw_Ticks){
				PWM_Yaw = Current_Ticks - Previous_Yaw_Ticks;
			}
			else{
				PWM_Yaw = 0xFFFF - Previous_Yaw_Ticks + Current_Ticks - 1;
			}
		}
	}
}

uint32_t constrain(uint32_t min, uint32_t max, uint32_t value){
	if(value > max){
		return max;
	}
	else if(value < min){
		return min;
	}
	return value;
}

void Error_Handler(void)
{
  __disable_irq();
  if(MOTOR_CAN_TURN){
	  HAL_TIMEx_PWMN_Stop(&htim2, TIM_CHANNEL_1);
	  HAL_TIMEx_PWMN_Stop(&htim2, TIM_CHANNEL_2);
	  HAL_TIMEx_PWMN_Stop(&htim2, TIM_CHANNEL_3);
	  HAL_TIMEx_PWMN_Stop(&htim2, TIM_CHANNEL_4);
  }

  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
