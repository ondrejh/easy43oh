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

#ifdef DEBUG
#include "uart.h"
#endif

// board (leds, button)
#define LED_INIT() {P1DIR|=0x41;P1OUT&=~0x41;}
#define LED_RED_ON() {P1OUT|=0x01;}
#define LED_RED_OFF() {P1OUT&=~0x01;}
#define LED_RED_SWAP() {P1OUT^=0x01;}
#define LED_GREEN_ON() {P1OUT|=0x40;}
#define LED_GREEN_OFF() {P1OUT&=~0x40;}
#define LED_GREEN_SWAP() {P1OUT^=0x40;}

// motor output pins
#define MOTOR_SLEEP_PIN 2
#define MOTOR_DIR_PIN 1
#define MOTOR_STEP_PIN 0
// motor defines
#define MOTOR_INIT() {P2DIR|=0x07;P2OUT&=~0x07;}
#define MOTOR_SLEEP() {P2OUT&=~(1<<MOTOR_SLEEP_PIN);}
#define MOTOR_AWAKE() {P2OUT|=(1<<MOTOR_SLEEP_PIN);}
#define MOTOR_DIR_FWD() {P2OUT|=(1<<MOTOR_DIR_PIN);}
#define MOTOR_DIR_BCK() {P2OUT&=~(1<<MOTOR_DIR_PIN);}
#define MOTOR_STEP() {P2OUT|=(1<<MOTOR_STEP_PIN);P2OUT&=~(1<<MOTOR_STEP_PIN);}

// hw depended init
void board_init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_1MHZ;		// Set DCO
	DCOCTL = CALDCO_1MHZ;

    MOTOR_INIT(); // motor
	LED_INIT(); // leds
}

// main program body
int main(void)
{
	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); 	// init oscilator and leds
	timer_init(); 	// init timer

	uart_init(); // init uart interface

    LED_GREEN_ON();
    MOTOR_DIR_FWD();
    MOTOR_AWAKE();

	while(1)
	{
	    MOTOR_STEP();
		__bis_SR_register(CPUOFF + GIE); // enter sleep mode (leave on timer interrupt)
	}

	return -1;
}
