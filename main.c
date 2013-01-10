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

// button pins
#define LPAD_BTN_PIN 3
#define FWD_BTN_PIN 4
#define BCK_BTN_PIN 5
#define FRONT_SW_PIN 3
#define REAR_SW_PIN 4
// buttons init
#define BUTTONS_INIT() {P1DIR&=~((1<<LPAD_BTN_PIN)|(1<<FWD_BTN_PIN)|(1<<BCK_BTN_PIN));\
                        P1OUT|=((1<<LPAD_BTN_PIN)|(1<<FWD_BTN_PIN)|(1<<BCK_BTN_PIN));\
                        P1REN|=((1<<LPAD_BTN_PIN)|(1<<FWD_BTN_PIN)|(1<<BCK_BTN_PIN));\
                        P2DIR&=~((1<<FRONT_SW_PIN)|(1<<REAR_SW_PIN));\
                        P2REN|=((1<<FRONT_SW_PIN)|(1<<REAR_SW_PIN));}
// button input functions
#define LPAD_BTN ((P1IN&(1<<LPAD_BTN_PIN))==0)
#define FWD_BTN  ((P1IN&(1<<FWD_BTN_PIN))!=0)
#define BCK_BTN  ((P1IN&(1<<BCK_BTN_PIN))!=0)
#define FRONT_SW ((P2IN&(1<<FRONT_SW_PIN))==0)
#define REAR_SW  ((P2IN&(1<<REAR_SW_PIN))==0)

// timeout for motor goto sleep mode [ticks]
#define SLEEP_TIMEOUT 20000

#define SEQV_SLEEP -1
#define SEQV_WAIT_BTN 0
#define SEQV_RUN_FORWARD 1

// hw depended init
void board_init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_8MHZ; // Set DCO
	DCOCTL  = CALDCO_8MHZ;

    MOTOR_INIT(); // motor outputs
	LED_INIT(); // leds
	BUTTONS_INIT(); // buttons
}

// main program body
int main(void)
{
    int16_t seqv = -1;
    uint16_t sleep_timer = 0;

	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); 	// init oscilator and leds

	timer_init(); // init timer

	uart_init(); // init uart interface

    t_motor motor; // init motor context
	motor_init(&motor);

    // test
    //while(!LPAD_BTN){};
    //motor_goto(&motor,8*200,SPEED_MAX/16);

	while(1)
	{
	    // sequential
	    switch (seqv)
	    {
	        case -1: // sleep
                if ((FWD_BTN)||(BCK_BTN))
                {
                    sleep_timer = SLEEP_TIMEOUT;
                    seqv++;
                }
                break;
	        case 0: // wait button (forward or backward)
                if (FWD_BTN)
                {
                    motor_run(&motor,SPEED_MAX/8);
                    seqv++;
                }
                else if (BCK_BTN)
                {
                    motor_run(&motor,-SPEED_MAX/8);
                    seqv++;
                }
                else if (sleep_timer==0)
                {
                    motor_sleep(&motor);
                    seqv--;
                }
                break;
            case 1: // run
                if (LPAD_BTN)
                {
                    motor_stop(&motor);
                    seqv++;
                }
                break;
            case 2: // wait released
                if ((!LPAD_BTN)&&(!BCK_BTN)&&(!FWD_BTN))
                {
                    sleep_timer = SLEEP_TIMEOUT;
                    seqv=0;
                }
                break;
            default:
                seqv=0;
                break;
	    }

	    // timer issue
	    while (ticks!=0)
	    {
	        ticks--;

	        if (sleep_timer) sleep_timer--;

	        motor_move(&motor);
	    }

		__bis_SR_register(CPUOFF + GIE); // enter sleep mode (leave on timer interrupt)
	}

	return -1;
}
