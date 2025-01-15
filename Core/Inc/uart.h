#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"

extern UART_HandleTypeDef huart1;

void MX_USART1_UART_Init(void);
void debug_uart(const char *message);

#endif /* INC_UART_H_ */
