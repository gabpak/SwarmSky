#ifndef INC_GPIO_HAL_H_
#define INC_GPIO_HAL_H_

#include "gpio.h"
#include "ISR_HAL.h"

typedef void (*gpio_isr_cb)(void);

class GPIObase{
public:
	GPIObase(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
	~GPIObase();

	static void init();
	uint16_t get_pin();

private:
	static bool isInit;

protected:
	GPIO_TypeDef* _GPIOx;
	uint16_t _GPIO_Pin;
};

// Dout
class Dout : public GPIObase{
public:
	Dout(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState state = GPIO_PIN_RESET);
	~Dout();

	void write(GPIO_PinState pinState);
	void toggle();
};



// Dint
class Din : public GPIObase{
public:
	Din(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
	~Din();

	GPIO_PinState read();
};

// DISR (for interrupts)
class DISR : public Din{
public:
	DISR(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
	~DISR();

	void set_isr_cb(gpio_isr_cb cb);
	void call_isr_cb();

	static void trigger_pin(uint16_t GPIO_Pin);

private:

	gpio_isr_cb _cb; // Pointer on a fonction of type "void foo(void)"
	static ISR<DISR> _ISR_LIST; // All the ISR of the program
};











#endif /* INC_GPIO_HAL_H_ */
