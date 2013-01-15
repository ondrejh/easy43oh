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

void restart_adc(uint8_t channel)
{
    switch (channel)
    {
        case 4:
            ADC10CTL1 = INCH_4;
            break;
        case 5:
            ADC10CTL1 = INCH_5;
            break;
        default:
            return;
    }
    ADC10CTL0 |= ENC + ADC10SC;
}

void start_adc(uint8_t channel)
{
    switch (channel)
    {
        case 4:
            ADC10CTL1 = INCH_4;
            break;
        case 5:
            ADC10CTL1 = INCH_5;
            break;
        default:
            return;
    }
    //ADC10CTL1 = INCH_10;
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REF2_5V + REFON + ADC10ON;
    ADC10AE0  |= 0x30;
    ADC10CTL0 |= ENC + ADC10SC;
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
