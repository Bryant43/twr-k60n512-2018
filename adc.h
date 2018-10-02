/*
 * adc1.h
 *
 */
 
#ifndef ADC1_H_
#define ADC1_H_

#include "MK60DZ10.h"

#define A                 0x0
#define B                 0x1

#define COCO_NOT          0x00

#define CALF_FAIL          ADC_SC3_CALF_MASK

#define AVGS_4             0x00
#define AVGS_8             0x01
#define AVGS_16            0x02
#define AVGS_32            0x03
//ADC configuration object
typedef struct ADC_Config{
	uint32_t CONFIG1;
	uint32_t CONFIG2;
	uint32_t COMPARE1;
	uint32_t COMPARE2;
	uint32_t STATUS1A;
	uint32_t STATUS1B;
	uint32_t STATUS2;
	uint32_t STATUS3;
	uint32_t PGA;
}*ADC_Config;
//Configuration for single-ended mode
void ADC1_Init16b(void);
uint16_t ADC1_Read16b(uint8_t channelNumber);
//Calibration for single-ended mode
uint8_t ADC_CalSingle(ADC_MemMapPtr adcmap);
void ADC_Config_Alt(ADC_MemMapPtr adcmap, ADC_Config ADC_CfgPtr);
void ConfigADC1(ADC_Config cfg);
#endif /* ADC1_H_ */
