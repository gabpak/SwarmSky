#include "main.h"
#include "clock.h"
#include "i2c.h"
#include "uart.h"
#include "gpio.h"
#include "mpu6050.h"

#include <string.h>
#include <stdio.h>

// MCU
MPU6050_t MPU6050;

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();

  while (MPU6050_Init(&hi2c1) == 1);

  while (1)
  {
	  char msg[64];
	  MPU6050_Read_All(&hi2c1, &MPU6050);
	  HAL_Delay(2);

	  sprintf(msg, "%f\n", MPU6050.KalmanAngleX);
	  debug_uart(msg);

	  if(MPU6050.KalmanAngleX > 45.0){
		  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		  HAL_Delay(250);
	  }
  }
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
