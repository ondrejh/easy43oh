/*
 * adc.h .. adc module header file
 *
 */

#ifndef __ADC_H__
#define __ADC_H__

#include <inttypes.h>

// read buffer function
uint16_t read_adc(uint8_t chnl);

// module initialization
void adc_init(void);

#endif
