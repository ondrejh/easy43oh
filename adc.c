/*
 * adc.c
 *
 * Created on: 7.9.2012
 *     Author: ondrejh.ck@gmail.com
 *
 * Description:
 * After initialization it periodically scans ADC4 and ADC5 and stores the value into buffer.
 * Buffer is accesable using ReadAdc function.
 *
 */

// include section
#include <msp430g2553.h>

#include "adc.h"

#define LED_RED_SWAP() {P1OUT^=0x01;}

// 2 channels with 2 buffered values
uint16_t adcbuf[4] = {0,0,0,0};
uint8_t adcbufptr = 0;

uint16_t read_adc(uint8_t chnl)
{
    uint8_t bufptr = (adcbufptr/2 + 2 + chnl)&0x03;
    return adcbuf[bufptr];
}

// init adc
void adc_init(void)
{
	// init adc4 measurement
	ADC10CTL0 = ADC10ON + ADC10IE + REFON + REF2_5V + SREF_1 + ADC10SR + ADC10SHT_1;
	//ADC10CTL0 = SREF_1 + ADC10SHT_3 + REF2_5V + ADC10IE + REFON + ADC10ON; // ref 2.5V
	ADC10CTL1 = ADC10SSEL_3 + ADC10DIV_7 + INCH_4; // adc10 int.osc., div 4, channel 4
	ADC10AE0 |= 0x30; // PA.4,5 ADC option select
    ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
    static uint16_t chnl = 4;

    adcbuf[adcbufptr++] = ADC10MEM;
    adcbufptr&=0x03;

    if (chnl==4)
    {
        ADC10CTL1 = ADC10SSEL_3 + ADC10DIV_7 + INCH_5;
        chnl=5;
    }
    else
    {
        ADC10CTL1 = ADC10SSEL_3 + ADC10DIV_7 + INCH_4;
        chnl=4;
    }

    // restart conversion
    ADC10CTL0 |= ENC + ADC10SC;
    LED_RED_SWAP();
}
