/*
 * adc.c
 *
 * Created on: 15.1.2013
 *     Author: ondrejh.ck@email.cz
 *
 * Description:
 *
 */

// include section
#include <msp430g2553.h>

#include "adc.h"

#define LED_RED_SWAP() {P1OUT^=0x01;}

void start_adc(uint16_t channel)
{
    ADC10CTL0 &= ~ENC;
    ADC10CTL1 = (channel<<12);
    ADC10CTL0 |= ENC + ADC10SC;
}

void init_adc(void)//uint16_t channel)
{
    ADC10AE0  |= 0x30; // ADC4, ADC5
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REF2_5V + REFON + ADC10ON;
}

bool adc_done(void)
{
    if (ADC10CTL1 & ADC10BUSY) return false;
    return true;
}

uint16_t read_adc(void)
{
    LED_RED_SWAP();
    return ADC10MEM;
}
