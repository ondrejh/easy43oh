//******************************************************************************
// Launchpad test application. EasyDriver stepper motor module
//
// author: Ondrej Hejda
// date:   9.1.2013
//
// hardware: MSP430G2553 (launchpad)
//
//                MSP4302553
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |           P1.1,2| --> UART (debug output 9.6kBaud)
//            |                 |
//            |             P1.0| --> RED LED (active high)
//            |             P1.6| --> GREEN LED (active high)
//            |                 |
//            |                 |      - easydriver -
//            |             P2.0| --> | STEP         |
//            |             P2.1| --> | DIRECTION    | - /4/ - motor
//            |             P2.2| --> | SLEEP        |
//            |                 |      --------------
//            |                 |

//******************************************************************************

#define DEBUG

// include section
#include <msp430g2553.h>
#include "timer.h"
#include "uart.h"
#include "easydrv.h"

// board (leds, button)
#define LED_INIT() {P1DIR|=0x41;P1OUT&=~0x41;}
#define LED_RED_ON() {P1OUT|=0x01;}
#define LED_RED_OFF() {P1OUT&=~0x01;}
#define LED_RED_SWAP() {P1OUT^=0x01;}
#define LED_GREEN_ON() {P1OUT|=0x40;}
#define LED_GREEN_OFF() {P1OUT&=~0x40;}
#define LED_GREEN_SWAP() {P1OUT^=0x40;}

// hw depended init
void board_init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_8MHZ; // Set DCO
	DCOCTL  = CALDCO_8MHZ;

    MOTOR_INIT(); // motor outputs
	LED_INIT(); // leds
}

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); 	// init oscilator and leds

	timer_init(); // init timer

	uart_init(); // init uart interface

    t_motor motor; // init motor context
	motor_init(&motor);

    // test
    motor_run(&motor,SPEED_MAX/16);

	while(1)
	{
	    if (ticks!=0) // timer issue
	    {
	        ticks--;

	        motor_move(&motor);
	    }

		__bis_SR_register(CPUOFF + GIE); // enter sleep mode (leave on timer interrupt)
	}

	return -1;
}
