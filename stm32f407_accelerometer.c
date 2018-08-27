/* Created on Wed 08.11.2017
   @author Elman Karimli */



#include "stm32f4xx.h"	// 
int main(){
	// Standart Part
  // Declarations
SPI_HandleTypeDef SPI_Params; // Declaring the structure handle for the parameters of SPI1
GPIO_InitTypeDef GPIOA_Params; // Declaring the structure handle for the parameters of GPIOA
GPIO_InitTypeDef GPIOE_Params; // Declaring the structure handle for the parameters of GPIOE
uint8_t data_to_send[1]; //Declaring an array to store the required LIS3DSH register address in. It has a single element since we will only be accessing a single address in each SPI transaction.
uint16_t data_size=1; //Declaring a variable that specifies that only a single address is accessed in each transaction.
uint32_t data_timeout=1000; //Setting a maximum time to wait for the SPI transaction to complete in - this mean that our program won’t freeze if there is a problem with the SPI communication channel.
uint8_t Who_am_I; //Declaring the variable to store the who_am_I register value in
uint8_t CTRL_REG4;
uint8_t Z_Reg_H; //Declaring the variable to store the z-axis MS 8-bits in
uint8_t Z_Reg_L; //Declaring the variable to store the z-axis LS 8-bits in
int16_t Z_Accn; //Declaring the variable to store the z-axis acceleration in - note that this is a signed 16-bit number

uint8_t y_Reg_H; //Declaring the variable to store the y-axis MS 8-bits in
uint8_t y_Reg_L; //Declaring the variable to store the y-axis LS 8-bits in
int16_t y_Accn; //Declaring the variable to store the y-axis acceleration in - note that this is a signed 16-bit number

uint8_t x_Reg_H; //Declaring the variable to store the x-axis MS 8-bits in
uint8_t x_Reg_L; //Declaring the variable to store the x-axis LS 8-bits in
int16_t x_Accn; //Declaring the variable to store the x-axis acceleration in - note that this is a signed 16-bit number

GPIO_InitTypeDef GPIOE_Params_I; // Declaring the structure handle for the parameters of the interrupt pin on GPIOE

//  Initializing pin 3 of GPIOE
RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; //Enabling the clock for GPIOE
GPIOE_Params.Pin = GPIO_PIN_3; // Enabling pin 3
GPIOE_Params.Mode = GPIO_MODE_OUTPUT_PP; //Selecting normal push-pull mode
GPIOE_Params.Speed = GPIO_SPEED_FAST; //Selecting fast speed
GPIOE_Params.Pull = GPIO_PULLUP; //Selecting pull-up activation
HAL_GPIO_Init(GPIOE, &GPIOE_Params); // Setting GPIOE into the modes specified in GPIOE_Params
GPIOE->BSRR = GPIO_PIN_3; //Setting the serial port enable pin CS high (idle) 
__HAL_SPI_ENABLE(&SPI_Params); //Enabling the SPI

//  Intializing GPIOE pin 0 for the interrupt
GPIOE_Params_I.Pin = GPIO_PIN_0; // Selecting pin 0
GPIOE_Params_I.Mode = GPIO_MODE_IT_RISING; // Selecting the interrupt mode and configures the interrupt to be signalled on a rising edge (low to high transition)
GPIOE_Params_I.Speed = GPIO_SPEED_FAST; //Selecting fast speed
HAL_GPIO_Init(GPIOE, &GPIOE_Params_I); // Setting GPIOE into the modes specified in GPIOE_Params_I

	
// Initialising SPI 
RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;//Enabling the clock for SPI1
SPI_Params.Instance = SPI1; // Selecting which SPI interface to use
SPI_Params.Init.Mode = SPI_MODE_MASTER;// Setting the STM32F407 to act as the master
SPI_Params.Init.NSS = SPI_NSS_SOFT; // Setting the slave to be controlled by software
SPI_Params.Init.Direction = SPI_DIRECTION_2LINES; // Setting the SPI to fullduplex
SPI_Params.Init.DataSize = SPI_DATASIZE_8BIT; // Setting the data packet size to 8-bit
SPI_Params.Init.CLKPolarity = SPI_POLARITY_HIGH; // Setting the idle polarity for the clock line to high
SPI_Params.Init.CLKPhase = SPI_PHASE_2EDGE; // Setting the data line to change on the second transition of the clock line
SPI_Params.Init.FirstBit = SPI_FIRSTBIT_MSB; // Setting the transmission to MSB first
SPI_Params.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32; // Setting the clock prescaler to divide the main APB2 clock (previously set to 84MHz) by 32 to give a SPI clock of 2.625MHz, which is less the maximum value of 10MHz for the SPI.
HAL_SPI_Init(&SPI_Params); // Configuring the SPI using the specified parameters 

// Initialising 5-7 pins in GPIAO
RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; //Enabling the clock for GPIOA
GPIOA_Params.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7; // Selecting pins 5,6 and 7
GPIOA_Params.Alternate = GPIO_AF5_SPI1; //Selecting alternate function 5 which corresponds to SPI1
GPIOA_Params.Mode = GPIO_MODE_AF_PP; //Selecting alternate function push-pull mode
GPIOA_Params.Speed = GPIO_SPEED_FAST; //Selecting fast speed

GPIOA_Params.Pull = GPIO_NOPULL; //Selecting no pull-up or pull-down activation
HAL_GPIO_Init(GPIOA, &GPIOA_Params); // Setting GPIOA into the modes specified in GPIOA_Params



// Initialize GPIO Port for LEDs
RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; // Enabling Port D clock
GPIOD->MODER |= GPIO_MODER_MODER12_0; // Port 12 output - green LED
GPIOD->MODER |= GPIO_MODER_MODER13_0; // Port 13 output - orange LED
GPIOD->MODER |= GPIO_MODER_MODER14_0; // Port 14 output - Red LED
GPIOD->MODER |= GPIO_MODER_MODER15_0; // Port 15 output - blue LED


// Writing value to control register

data_to_send[0] = 0x20; // Address for Control register on LIS3DSH
GPIOE->BSRR = GPIO_PIN_3<<16; // Setting the SPI communication enable line low to initiate communication
HAL_SPI_Transmit(&SPI_Params,data_to_send,data_size,data_timeout); // Send the address of the register to be read on the LIS3DSH
data_to_send[0] = 0x13; // Setting a blank address because we are waiting to receive data
HAL_SPI_Transmit(&SPI_Params,data_to_send,data_size,data_timeout); // Send the address of the register to be read on the LIS3DSH
GPIOE->BSRR = GPIO_PIN_3; // Setting the SPI communication enable line high to signal the end of the communication process

//Task 4

// Writing a new value to control register 3 of the accelerometer to configure the interrupts 
data_to_send[0] = 0x23; // Addressing for control register 3 on the LIS3DSH

GPIOE->BSRR = GPIO_PIN_3<<16; // Setting the SPI communication enable line low to initiate communication
HAL_SPI_Transmit(&SPI_Params,data_to_send,data_size,data_timeout); // Send the address of the register to be read on the LIS3DSH
data_to_send[0] = 0xC8; // Enabling DRDY connected to Int1, sets Int1 active to high, enables int1
HAL_SPI_Transmit(&SPI_Params,data_to_send,data_size,data_timeout); // Sending the new register value to the LIS3DSH through the SPI channel
GPIOE->BSRR = GPIO_PIN_3; // Setting the SPI communication enable line high to signal the end of the communication process




for(;;){
	
if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0)==SET){ // Checking to see if the interrupt line has been set
	
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0); // Clearing the interrupt before proceeding to service the interrupt 
	
	
// Reading the value from the MSB (H) x-axis data register of the accelerometer

data_to_send[0] = 0x80|0x29; // Addressing for the MSB z-axis (H) data register on the LIS3DSH
GPIOE->BSRR = GPIO_PIN_3<<16; // Setting the SPI communication enable line low to initiate communication
HAL_SPI_Transmit(&SPI_Params,data_to_send,data_size,data_timeout); // Sending the address of the register to be read on the LIS3DSH
data_to_send[0] = 0x00; // Setting a blank address because we are waiting to receive data
HAL_SPI_Receive(&SPI_Params,data_to_send,data_size,data_timeout); // Getting the data from the LIS3DSH through the SPI channel
GPIOE->BSRR = GPIO_PIN_3; // Setting the SPI communication enable line high to signal the end of the communication process
x_Reg_H = *SPI_Params.pRxBuffPtr; // Reading the data from the SPI buffer substructure into our internal variable.

  // Reading the value from the LSB (L) z-axis data register of the accelerometer 

	data_to_send[0] = 0x80| 0x28; // Address for the LSB (L) z-axis data register of the LIS3DSH
GPIOE->BSRR = GPIO_PIN_3<<16; // Setting the SPI communication enable line low to initiate communication
HAL_SPI_Receive(&SPI_Params,data_to_send,data_size,data_timeout);

	
	// Sending the address of the register to be read on the LIS3DSH  
data_to_send[0] = 0x00; // Setting a blank address because we are waiting to receive data
HAL_SPI_Transmit(&SPI_Params,data_to_send,data_size,data_timeout); // Get the data from the LIS3DSH through the SPI channel
GPIOE->BSRR = GPIO_PIN_3; // Setting the SPI communication enable line high to signal the end of the communication process
x_Reg_L = *SPI_Params.pRxBuffPtr; // Reading the data from the SPI buffer substructure into our internal variable.
x_Accn = (x_Reg_H<<8)|x_Reg_L; // Combined the MSB (H) and LSB (L) 8-bit registers into a single 16-bit value



  // Reading the value from the MSB (H) y-axis data register of accelerometer

data_to_send[0] = 0x80|0x2B; // Address for the MSB y-axis (H) data register on the LIS3DSH
GPIOE->BSRR = GPIO_PIN_3<<16; // Set the SPI communication enable line low to initiate communication
HAL_SPI_Transmit(&SPI_Params,data_to_send,data_size,data_timeout); // Send the address of the register to be read on the LIS3DSH
data_to_send[0] = 0x00; // Set a blank address because we are waiting to receive data
HAL_SPI_Receive(&SPI_Params,data_to_send,data_size,data_timeout); // Get the data from the LIS3DSH through the SPI channel
GPIOE->BSRR = GPIO_PIN_3; // Set the SPI communication enable line high to signal the end of the communication process
y_Reg_H = *SPI_Params.pRxBuffPtr; // Read the data from the SPI buffer substructure into our internal variable.

 // Reading the value from the LSB (L) z-axis data register of the LIS3DSH 

data_to_send[0] = 0x80| 0x2A; // Address for the LSB (L) z-axis data register of the LIS3DSH
GPIOE->BSRR = GPIO_PIN_3<<16; // Set the SPI communication enable line low to initiate communication
HAL_SPI_Receive(&SPI_Params,data_to_send,data_size,data_timeout);

 // Sending the address of the register to be read on the accelerometer 

data_to_send[0] = 0x00; // Set a blank address because we are waiting to receive data
HAL_SPI_Transmit(&SPI_Params,data_to_send,data_size,data_timeout); // Get the data from the LIS3DSH through the SPI channel
GPIOE->BSRR = GPIO_PIN_3; // Setting the SPI communication enable line high to signal the end of the communication process
y_Reg_L = *SPI_Params.pRxBuffPtr; // Reading the data from the SPI buffer substructure into our internal variable.
y_Accn = (y_Reg_H<<8)|y_Reg_L; // Combined the MSB (H) and LSB (L) 8-bit registers into a single 16-bit value

//Z- axis ignored because of avoiding enabling the led after 90 degree 
//X-axis

if((x_Accn&0x8000) == 0x8000){ 

GPIOD->BSRR |= (1<<12); 
GPIOD->BSRR |= (1<<(14+16)); 
	
		if((y_Accn&0x8000) == 0x8000){ 
		GPIOD->BSRR |= (1<<15); //
		GPIOD->BSRR |= (1<<(13+16)); 
		}
		else if ((y_Accn&0xffff)== 0x0000){
		GPIOD->BSRR |= (1<<(13+16)); 
		GPIOD->BSRR |= (1<<(15+16)); 
		}
		else{
		GPIOD->BSRR |= (1<<13);      
		GPIOD->BSRR |= (1<<(15+16)); 
		}		
}
//Y-axis
else if ((x_Accn&0xffff) == 0x0000){
	
	GPIOD->BSRR |= (1<<(12+16)); 
	GPIOD->BSRR |= (1<<(14+16)); 
	
	  if((y_Accn&0x8000) == 0x8000){ 
		GPIOD->BSRR |= (1<<15); 
		GPIOD->BSRR |= (1<<(13+16)); 
		}
		else if ((y_Accn&0xffff)== 0x0000){
		GPIOD->BSRR |= (1<<(13+16)); 
		GPIOD->BSRR |= (1<<(15+16)); 
		}
		else{
		GPIOD->BSRR |= (1<<13); //  
		GPIOD->BSRR |= (1<<(15+16)); 
		}
}
else {
	
	GPIOD->BSRR |= (1<<14); 
	GPIOD->BSRR |= (1<<(12+16)); 
	
	  if((y_Accn&0x8000) == 0x8000){ 
		GPIOD->BSRR |= (1<<15); 
		GPIOD->BSRR |= (1<<(13+16)); 
		}
		else if ((y_Accn&0xffff)== 0x0000){
		GPIOD->BSRR |= (1<<(13+16)); 
		GPIOD->BSRR |= (1<<(15+16)); 
		}
		else{
		GPIOD->BSRR |= (1<<13); 
		GPIOD->BSRR |= (1<<(15+16)); 
		}	
}



}




}
}
