/*
 * adc1.c
 *
 */
 
#include "adc.h"

ADC_Config cfg16b;
enum ADVI{ADIV_2=0x0,ADIV_4=0x2,ADIV_8=0x3};
enum ADLSMP{ADLSMP_SHORT,ADLSMP_LONG};
enum MODE{MODE_8,MODE_12,MODE_10,MODE_16};
enum ADICLK{ADICLK_BUS,ADICLK_BUS_BY2,ADICLK_ALTCLK,ADICLK_ADACK};

enum MUXSEL{MUXSEL_ADCA,MUXSEL_ADCB};
enum ADACKEN{ADACKEN_DISABLED,ADACKEN_ENABLED};
enum ADHSC{NORMAL,HISPEED};
enum ADLSTS{ADLSTS_20,ADLSTS_16,ADLSTS_10,ADLSTS_6};
enum AVGS{AVGS_40,AVGS_80,AVGS_160,AVGS_320};
enum AVGE{AVGE_DISABLED,AVGE_ENABLED};
enum PGA_PGAG {PGAG_1,PGAG_2,PGAG_4,PGAG_8,PGAG_16,PGAG_32,PGAG_64};

void ConfigADC1(ADC_Config cfg){
	//by default in normal power mode
	cfg->CONFIG1 = ADC_CFG1_ADIV(ADIV_4)//divide clock by 8
			| ADLSMP_LONG//long sample time
			| ADC_CFG1_MODE(MODE_16) //16-bit mode
			| ADC_CFG1_ADICLK(ADICLK_BUS);//bus clock as source
	cfg->CONFIG2 = MUXSEL_ADCA //multiplex ADxxa channels
			| (((uint32_t)(((uint32_t)(ADACKEN_DISABLED))<<ADC_CFG2_ADACKEN_SHIFT))&ADC_CFG2_ADACKEN_MASK) //disable asynchronous clock output
			| (((uint32_t)(((uint32_t)(HISPEED))<<ADC_CFG2_ADHSC_SHIFT))&ADC_CFG2_ADHSC_MASK)
			| ADC_CFG2_ADLSTS(ADLSTS_20) ;
	cfg->COMPARE1 = 0x1234u ; // can be anything
	cfg->COMPARE2 = 0x5678u ; // can be anything // since not using // compare feature
	cfg->STATUS2 = ADC_SC2_REFSEL(0);//external reference
	//by default cal bit is 0, single short mode
	cfg->STATUS3 = ((uint32_t)((((uint32_t)(AVGE_ENABLED))<<ADC_SC3_AVGE_SHIFT))&ADC_SC3_AVGE_MASK)
			| ADC_SC3_AVGS(AVGS_320);
	//PGA disabled
	cfg->PGA = ADC_PGA_PGAG(PGAG_64);//PGA gain=2^64
	//conversion complete interrupt disabled by default AIEN
	//DIFF=0 by default, single ended mode
	cfg->STATUS1A = ADC_SC1_ADCH(31);//select channel 31 to disable module
	cfg->STATUS1B = ADC_SC1_ADCH(31);//same
	ADC_Config_Alt(ADC1_BASE_PTR,cfg);
}
// Configure ADC1 16bit single ended mode
void ADC1_Init16b(void){
    SIM_SCGC3 |= SIM_SCGC3_ADC1_MASK; //Gate the peripheral clock to ADC1
    ADC1_CFG1 = 0x00;//clear register to reset
    ADC1_CFG1 |= ADC_CFG1_ADIV(ADIV_4);//divide the clock by 4
    ADC1_CFG1 |= ADC_CFG1_MODE(MODE_16);//select 16 bit mode
    ADC1_SC2 &= ~(ADC_SC2_REFSEL_MASK);//external reference
    ADC1_SC1A |= ADC_SC1_ADCH(31);//disable the module
}

uint16_t ADC1_Read16b(uint8_t channelNumber){
	ADC1_SC1A = ADC_SC1_ADCH(channelNumber);// Write to ADCSC1 to start conversion
	while ((ADC1_SC2 & 0x80)); // Wait if the conversion is in progress
	while (!(ADC1_SC1A & 0x80)); // Wait until the conversion is complete
	return ADC1_RA; 
}

/******************************************************************************
	AUTO CAL ROUTINE  
	Calibrates the ADC1_ automatically.
	Required after reset and before a conversion is initiated
	16-bit single-ended mode
******************************************************************************/
uint8_t ADC_CalSingle(ADC_MemMapPtr adcmap){
  unsigned int cal_var;
  ADC_SC2_REG(adcmap) &=  ~ADC_SC2_ADTRG_MASK ; // Enable Software Conversion Trigger for Calibration Process
  ADC_SC3_REG(adcmap) &= ( ~ADC_SC3_ADCO_MASK & ~ADC_SC3_AVGS_MASK ); // set single conversion, clear avgs bitfield for next writing
  ADC_SC3_REG(adcmap) |= ( ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(AVGS_32) );  // Turn averaging ON and set at max value ( 32 )
  ADC_SC3_REG(adcmap) |= ADC_SC3_CAL_MASK ;      // Start CAL
  while ( (ADC_SC1_REG(adcmap,A) & ADC_SC1_COCO_MASK ) == COCO_NOT ); // Wait calibration end
  if ((ADC_SC3_REG(adcmap)& ADC_SC3_CALF_MASK) == CALF_FAIL ){
	  return(1);    // Check for Calibration fail error and return
  }
  // Calculate plus-side calibration for the overall conversion in single-ended mode.
  cal_var = 0x00;
  cal_var =  ADC_CLP0_REG(adcmap);
  cal_var += ADC_CLP1_REG(adcmap);
  cal_var += ADC_CLP2_REG(adcmap);
  cal_var += ADC_CLP3_REG(adcmap);
  cal_var += ADC_CLP4_REG(adcmap);
  cal_var += ADC_CLPS_REG(adcmap);
  cal_var = cal_var/2;
  cal_var |= 0x8000; // Set MSB
  ADC_PG_REG(adcmap) = ADC_PG_PG(cal_var);
  ADC_SC3_REG(adcmap) &= ~ADC_SC3_CAL_MASK ; /* Clear CAL bit */
  return(0);
}

void ADC_Config_Alt(ADC_MemMapPtr adcmap, ADC_Config ADC_CfgPtr){
 ADC_CFG1_REG(adcmap) = ADC_CfgPtr->CONFIG1;
 ADC_CFG2_REG(adcmap) = ADC_CfgPtr->CONFIG2;
 ADC_CV1_REG(adcmap)  = ADC_CfgPtr->COMPARE1;
 ADC_CV2_REG(adcmap)  = ADC_CfgPtr->COMPARE2;
 ADC_SC2_REG(adcmap)  = ADC_CfgPtr->STATUS2;
 ADC_SC3_REG(adcmap)  = ADC_CfgPtr->STATUS3;
 ADC_PGA_REG(adcmap)  = ADC_CfgPtr->PGA;
 ADC_SC1_REG(adcmap,A)= ADC_CfgPtr->STATUS1A;
 ADC_SC1_REG(adcmap,B)= ADC_CfgPtr->STATUS1B;
}

