/*
 * File: main.c
 *
 * Copyright (c) 2015 David Muriuki
 * see the LICENCE file
 */

#include "main.h"
uint8_t blink=0, data;
uint16_t adcval = 0;
float voltage=0.0;
uint8_t whole=0, frac=0;

int main(void){
	//initialize system
	SystemInit();
	//initialize ports
	gpio_init();
	//initialize & calibrate the ADC
	ADC1_Init16b();
	(void)ADC_CalSingle(ADC1_BASE_PTR);
	//initialize a serial port
	init_uart(UART5_BASE_PTR,periph_clk_khz,115200);
	puts((uint8_t *)"Hello world\r\n");
	//Loop forever
	while(1)
	{
		if(blink)
			toggle_LEDS();
		if(data_available()){
			data = uart_read();
			//parse the data from UART0
			// if data is a number 0-9, display it on LEDs
			if(data<=0x39 && data>=0x30){
				display(data-0x30);
				uartsend(data);
			}
			// if data is a 'v' or 'V'
			if(data==0x56 || data==0x76){
				//read the adc
				adcval = ADC1_Read16b(20);
				//calculate the voltage
				voltage = adcval * 3.3;
				voltage = voltage/65535;
				//format voltage as a string and send
				whole = ((uint8_t)voltage);
				frac = (voltage*100)-(whole*100);
				whole = whole+0x30;
				puts((uint8_t *)"\r\n");
				uartsend(whole);
				uartsend(0x2E);
				uartsend(((uint8_t)(frac/10))+0x30);
				uartsend(frac-((uint8_t)(frac/10)*10)+0x30);
				puts((uint8_t*)" Volts\r\n");
			}
		}
		delay();	
	}
}

/*
* display
*
* A function for displaying a 4 bit number in binary using the LEDs E1,E2,E3,E4
*
* Parameters:
* num - an 8-bit unsigned integer whose first 4 bits are displayed
*/
void display(uint8_t num){
	uint8_t bits[4]={11,28,29,10}, j=0;
	for(j=0;j<4;j++){
		//if((num>>j)&1){
		//	GPIOA_PDOR &= ~((uint32_t)(1<<bits[j]));
		//}else{
		//	GPIOA_PDOR |= (uint32_t)(1<<bits[j]);
		//}
		((num>>j)&1)?(GPIOA_PDOR &= ~(1<<bits[j])):(GPIOA_PDOR |= (1<<bits[j]));
	}
}

/*
	brief  Silly delay
*/
void delay(void)
{
  volatile unsigned int i,k;

  for(i=0; i<1000; i++)
  {
	for(k=0; k<1000; k++){
	 __asm__("nop");
	}
  }
}
