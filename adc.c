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

//#define LED_RED_SWAP() {P1OUT^=0x01;}


uint16_t ChBuff[2] = {0,0}; // read channels buffer
uint16_t ChnlPtr  = 4;      // channel pointer (what now, what next)
bool AdcValReady   = false; // values ready flag

// adc module initialization
void init_adc(void)
{
    ADC10AE0  |= 0x30; // ADC4, ADC5
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REF2_5V + REFON + ADC10ON + ADC10IE;
}

// start conversion (next channel)
void start_adc(void)
{
    AdcValReady = false;
    ADC10CTL0 &= ~ENC;
    ADC10CTL1 = (ChnlPtr<<12);
    ADC10CTL0 |= ENC + ADC10SC;
}

// return adc ready flag (says values are converted)
bool adc_ready(void)
{
    return AdcValReady;
}

// get values from buffer
uint16_t get_adc(uint8_t chnl)
{
    return (ChBuff[chnl&0x01]);
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
    //LED_RED_SWAP();
    ChBuff[ChnlPtr&0x0001]=ADC10MEM;
    ChnlPtr^=0x0001;

    if (ChnlPtr==5) start_adc();
    else AdcValReady=true;
}
