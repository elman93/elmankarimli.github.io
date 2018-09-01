/* Created on Sat 31.11.2017
   @author Elman Karimli */
   
/*The red LED will blink repeatedly on and off at 0.5s intervals until the blue user button is pressed. 
When the button is pressed the red LED will immediately stop blinking and turn off. The green LED will turn on continuously 
for the period of time that the button is held down. 
When the button is released the green LED turns off and the red LED re-starts blinking . */

 
#include "stm32f4xx.h"
int main(void)
{
	// Initializing
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;// Initializing GPIOD clock
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Initializing GPIOA BUTTON
	GPIOD->MODER |= GPIO_MODER_MODER12_0; // Enables GPIOD pin 12 output - green LED
	GPIOD->MODER |= GPIO_MODER_MODER14_0; // Enables GPIOD pin 14 output - green LED
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Initializing timer 2 clock
	TIM2->CR1 &= ~0x00000016; //Setting the counter as an upcounter
	TIM2->CR1 = 1<<3; /*Turn off repeat in CR1 - i.e. use one pulse mode*/
	TIM2->PSC = 8400-1; /*Prescaler value - the prescaler clock is running at 16MHz (the default value is
	set to the internal clock)*/
	TIM2->ARR = 5000-1; /*sets the value in the autoreload register*/ //0.5 second on/off delays
	TIM2->EGR = 1;
	for(;;)
	{
		if((GPIOA->IDR &=1)==1) // if the pushbutton is pushed
		{
			GPIOD->BSRR =1<<12; //turn on the green led
			GPIOD->BSRR=1<<(14+16); //turn off the red led
		} 
		else
		{
			GPIOD->BSRR=1<<(12+16); //Turn off the green led
			GPIOD->BSRR=1<<(14); //turn on red led
			TIM2->CR1|=0x00000001; // Setting up the counter
			while((TIM2->SR&0x0001)!=1){}; //TIM2 to implement a 1 second delay
			TIM2->SR&= 0x11111110; //Reseting the counter
			GPIOD->BSRR=1<<(14+16); //turn off the red led
			TIM2->CR1 |=0x00000001; //Setting the counter
			while((TIM2->SR&0x0001)!=1){}; //TIM2 to implement a 1 second delay
			TIM2->SR&= 0x11111110; // Reseting the counter
		}
	}	
}
