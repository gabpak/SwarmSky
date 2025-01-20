#include "gpio_hal.h"

// Definition of the static variable _ISR_LIST
ISR<DISR> DISR::_ISR_LIST;

// Redefinition of __weak HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	DISR::trigger_pin(GPIO_Pin);
}

// GPIObase
bool GPIObase::isInit = false;

GPIObase::GPIObase(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) : _GPIOx(GPIOx), _GPIO_Pin(GPIO_Pin){
	init();
}
GPIObase::~GPIObase(){}

void GPIObase::init(){
	if(GPIObase::isInit){
		return;
	}

	MX_GPIO_Init();
	GPIObase::isInit = true;
}

uint16_t GPIObase::get_pin(){
	return _GPIO_Pin;
}


// Dout
Dout::Dout(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState state) : GPIObase(GPIOx, GPIO_Pin){
	write(state);
}
Dout::~Dout(){}

void Dout::write(GPIO_PinState pinState){
	HAL_GPIO_WritePin(_GPIOx, _GPIO_Pin, pinState);
}

void Dout::toggle(){
	HAL_GPIO_TogglePin(_GPIOx, _GPIO_Pin);
}


// Din
Din::Din(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) : GPIObase(GPIOx, GPIO_Pin){}
Din::~Din(){}

GPIO_PinState Din::read(){
	return HAL_GPIO_ReadPin(_GPIOx, _GPIO_Pin);
}


// DISR
DISR::DISR(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) : Din(GPIOx, GPIO_Pin), _cb(nullptr){
	DISR::_ISR_LIST.add(this);
}
DISR::~DISR(){
	DISR::_ISR_LIST.remove(this);
}

void DISR::set_isr_cb(gpio_isr_cb cb){
	if(cb == nullptr){
		return;
	}
	_cb = cb;
}

void DISR::call_isr_cb(){
	if(_cb == nullptr){
		return;
	}
	_cb();
}

void DISR::trigger_pin(uint16_t GPIO_Pin){
	for(int i = 0; i < DISR::_ISR_LIST.size(); i++){
		uint16_t pin = DISR::_ISR_LIST.get(i)->get_pin();
		if(pin == GPIO_Pin){
			DISR::_ISR_LIST.get(i)->call_isr_cb();
		}
	}
}











