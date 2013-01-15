/*
 * adc.h .. adc module header file
 *
 */

#ifndef __ADC_H__
#define __ADC_H__

#include <inttypes.h>
#include <stdbool.h>

void start_adc(void);
bool adc_ready(void);
uint16_t get_adc(uint8_t chnl);

void init_adc(void);

#endif
