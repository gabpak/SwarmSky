#include "main.h"
#include "gpio_hal.h"
#include "usart.h"
#include "tim.h"
#include "i2c.h"
#include "mpu6050.h"

#include <stdio.h>

#define MIN_PWM_INPUT_RECEIVER 1000 	// 1ms (5% de 20ms)
#define MAX_PWM_INPUT_RECEIVER 2000 // 2ms (10% de 20ms)
#define MAX_PWM_COUNTER 3600 // 3600 sur 36000 = 10%
#define MIN_PWM_COUNTER 1800 // 1800 sur 36000 = 5%

void SystemClock_Config(void);
void PWM_Roll_Callback();
void PWM_Pitch_Callback();
void PWM_Thrust_Callback();
void PWM_Yaw_Callback();
void PWM_Killswitch_Callback();
uint32_t constrain(uint32_t min, uint32_t max, uint32_t value);

Dout PinOut_13(LED_13_GPIO_Port, LED_13_Pin, GPIO_PIN_RESET);
DISR SW1(PWM_Roll_GPIO_Port, PWM_Roll_Pin); // PWM Roll (PA8)
DISR SW2(PWM_Pitch_GPIO_Port, PWM_Pitch_Pin); // PWM Pitch (PA5)
DISR SW3(PWM_Thrust_GPIO_Port, PWM_Thrust_Pin); // PWM Thrust (PA6)
DISR SW4(PWM_Yaw_GPIO_Port, PWM_Yaw_Pin); // PWM Yaw (PA7)
DISR SW5(PWM_Killswitch_GPIO_Port, PWM_Killswitch_Pin); // PWM Yaw (PA7)

// PWM From the receiver
volatile uint32_t PWM_Roll = 0;
volatile uint32_t PWM_Pitch = 0;
volatile uint32_t PWM_Thrust = 0;
volatile uint32_t PWM_Yaw = 0;
volatile uint32_t PWM_Killswitch = 0;

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
  MX_USART1_UART_Init(); 	// usart
  MX_I2C1_Init();			// i2c
  MX_TIM1_Init();			// Timer for the measures of the PWM
  MX_TIM2_Init();			// Timer for the generation of the PWM


  // Starting the timer for the WPM measures
  HAL_TIM_Base_Start(&htim1);
  __HAL_TIM_SET_COUNTER(&htim1,0);
  SW1.set_isr_cb(PWM_Roll_Callback); // Roll
  SW2.set_isr_cb(PWM_Pitch_Callback); // Pitch
  SW3.set_isr_cb(PWM_Thrust_Callback); // Thrust
  SW4.set_isr_cb(PWM_Yaw_Callback); // Yaw
  SW5.set_isr_cb(PWM_Killswitch_Callback); // Kill_Switch

  // PWM Generator
  HAL_TIM_Base_Start(&htim2);
  __HAL_TIM_SET_COUNTER(&htim2,0);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

  // UART
  char buffer[64];

  // Sending PWM at 0% to the ESC
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0); // Gestion du PWM avec le Thrust
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0); // Gestion du PWM avec le Thrust
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0); // Gestion du PWM avec le Thrust
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0); // Gestion du PWM avec le Thrust

  PinOut_13.toggle();
  HAL_Delay(1000);
  PinOut_13.toggle();
  HAL_Delay(1000);
  PinOut_13.toggle();
  HAL_Delay(1000);
  PinOut_13.toggle();

  // mpu6050
  MPU6050_t MPU6050;
  while(MPU6050_Init(&hi2c1) == 1);

  while (1)
  {
	  MPU6050_Read_All(&hi2c1, &MPU6050);

	  if(!KILL_SWITCH){
		  PWM_CHANNEL3_PERIOD = (PWM_Thrust - 10) * 1800/1000;
		  PWM_CHANNEL3_PERIOD = constrain(MIN_PWM_COUNTER, MAX_PWM_COUNTER, PWM_CHANNEL3_PERIOD);
	  }
	  else{
		  PWM_CHANNEL1_PERIOD = 0;
		  PWM_CHANNEL2_PERIOD = 0;
		  PWM_CHANNEL3_PERIOD = 0;
		  PWM_CHANNEL4_PERIOD = 0;
		  HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
		  HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
		  HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3);
		  HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);

		  Error_Handler(); // On rend impossible de le remettre en route après un kill switch
	  }

	  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, PWM_CHANNEL3_PERIOD); // Gestion du PWM avec le Thrust
	  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, PWM_CHANNEL3_PERIOD); // Gestion du PWM avec le Thrust
	  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, PWM_CHANNEL3_PERIOD); // Gestion du PWM avec le Thrust
	  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, PWM_CHANNEL3_PERIOD); // Gestion du PWM avec le Thrust

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

// PWM Callbacks
void PWM_Roll_Callback(){
	uint32_t Current_Ticks = __HAL_TIM_GET_COUNTER(&htim1);
	static uint32_t Previous_Ticks = 0;

	if(SW1.read() == GPIO_PIN_SET){
		Previous_Ticks = Current_Ticks;  // saving the currents Ticks at the High State
	}
	else{
		if(Current_Ticks > Previous_Ticks){
			PWM_Roll = Current_Ticks - Previous_Ticks;
		}
		else{ // Timer had a reset
			PWM_Roll = 0xFFFF - Previous_Ticks + Current_Ticks - 1;
		}
	}
}

void PWM_Pitch_Callback(){
	uint32_t Current_Ticks = __HAL_TIM_GET_COUNTER(&htim1);
	static uint32_t Previous_Ticks = 0;

	if(SW2.read() == GPIO_PIN_SET){
		Previous_Ticks = Current_Ticks;  // saving the currents Ticks at the High State
	}
	else{
		if(Current_Ticks > Previous_Ticks){
			PWM_Pitch = Current_Ticks - Previous_Ticks;
		}
		else{ // Timer had a reset
			PWM_Pitch = 0xFFFF - Previous_Ticks + Current_Ticks - 1;
		}
	}
}

void PWM_Thrust_Callback(){
	uint32_t Current_Ticks = __HAL_TIM_GET_COUNTER(&htim1);
	static uint32_t Previous_Ticks = 0;

	if(SW3.read() == GPIO_PIN_SET){
		Previous_Ticks = Current_Ticks;  // saving the currents Ticks at the High State
	}
	else{
		if(Current_Ticks > Previous_Ticks){
			PWM_Thrust = Current_Ticks - Previous_Ticks;
		}
		else{ // Timer had a reset
			PWM_Thrust = 0xFFFF - Previous_Ticks + Current_Ticks - 1;
		}
	}
}

void PWM_Yaw_Callback(){
	uint32_t Current_Ticks = __HAL_TIM_GET_COUNTER(&htim1);
	static uint32_t Previous_Ticks = 0;

	if(SW4.read() == GPIO_PIN_SET){
		Previous_Ticks = Current_Ticks;  // saving the currents Ticks at the High State
	}
	else{
		if(Current_Ticks > Previous_Ticks){
			PWM_Yaw = Current_Ticks - Previous_Ticks;
		}
		else{ // Timer had a reset
			PWM_Yaw = 0xFFFF - Previous_Ticks + Current_Ticks - 1;
		}
	}
}

void PWM_Killswitch_Callback(){
	uint32_t Current_Ticks = __HAL_TIM_GET_COUNTER(&htim1);
	static uint32_t Previous_Ticks = 0;

	if(SW5.read() == GPIO_PIN_SET){
		Previous_Ticks = Current_Ticks;  // saving the currents Ticks at the High State
	}
	else{
		if(Current_Ticks > Previous_Ticks){
			PWM_Killswitch = Current_Ticks - Previous_Ticks;
		}
		else{ // Timer had a reset
			PWM_Killswitch = 0xFFFF - Previous_Ticks + Current_Ticks - 1;
		}
	}

	if(PWM_Killswitch > 1500){
		KILL_SWITCH = true;
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
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
