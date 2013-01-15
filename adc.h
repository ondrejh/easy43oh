/*
 * adc.h .. adc module header file
 *
 */

#ifndef __ADC_H__
#define __ADC_H__

#include <inttypes.h>
#include <stdbool.h>

void start_adc(uint16_t channel);

void init_adc(void);
bool adc_done(void);
uint16_t read_adc(void);

#endif
