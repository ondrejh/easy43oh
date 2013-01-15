/*
 * adc.h .. adc module header file
 *
 */

#ifndef __ADC_H__
#define __ADC_H__

#include <inttypes.h>
#include <stdbool.h>

void restart_adc(uint8_t channel);

void start_adc(uint8_t channel);
bool adc_done(void);
uint16_t read_adc(void);

#endif
