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
//        /|\ |                 |
//         |  |              XIN|-
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
//            |             P2.3| --> BUTTON STOP
//            |             P2.4| --> BUTTON FORWARD
//            |             P2.5| --> BUTTON BACKWARD
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
#define BTN_STOP_PIN 3
#define BTN_FORWARD_PIN 4
#define BTN_BACKWARD_PIN 5
// buttons init
#define BUTTONS_INIT() {P2DIR&=~((1<<BTN_STOP_PIN)|(1<<BTN_FORWARD_PIN)|(1<<BTN_BACKWARD_PIN));\
                        P2OUT|=((1<<BTN_STOP_PIN)|(1<<BTN_FORWARD_PIN)|(1<<BTN_BACKWARD_PIN));\
                        P2REN|=((1<<BTN_STOP_PIN)|(1<<BTN_FORWARD_PIN)|(1<<BTN_BACKWARD_PIN));}
// button input functions
#define BTN_STOP     ((P2IN&(1<<BTN_STOP_PIN))==0)
#define BTN_FORWARD  ((P2IN&(1<<BTN_FORWARD_PIN))==0)
#define BTN_BACKWARD ((P2IN&(1<<BTN_BACKWARD_PIN))==0)

// timeout for motor goto sleep mode [ticks]
#define SLEEP_TIMEOUT 20000 // 2s
#define BUTTON_RELEASE_TIMEOUT 1000 // 100ms

#define SEQV_SLEEP -1
#define SEQV_WAIT_BTN 0
#define SEQV_RUN_FORWARD 1
#define SEQV_RUN_BACKWARD 2
#define SEQV_WAIT_BTN_RELEASE 3

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
    uint16_t main_timer = 0;

	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT

	board_init(); 	// init oscilator and leds

	timer_init(); // init timer

	uart_init(); // init uart interface

    t_motor motor; // init motor context
	motor_init(&motor);

    // main loop
	while(1)
	{
	    // sequential
	    switch (seqv)
	    {
	        case SEQV_SLEEP: // sleep
                if ((BTN_FORWARD)||(BTN_BACKWARD))
                {
                    main_timer = SLEEP_TIMEOUT;
                    seqv++;
                }
                break;
	        case SEQV_WAIT_BTN: // wait button (forward or backward)
                if (BTN_FORWARD)
                {
                    motor_run(&motor,SPEED_MAX/8);
                    seqv=1;
                }
                else if (BTN_BACKWARD)
                {
                    motor_run(&motor,-SPEED_MAX/8);
                    seqv=2;
                }
                else if (main_timer==0)
                {
                    motor_sleep(&motor);
                    seqv--;
                }
                break;
            case SEQV_RUN_FORWARD: // run forward
                if (BTN_STOP||BTN_BACKWARD)
                {
                    motor_stop(&motor);
                    main_timer = BUTTON_RELEASE_TIMEOUT;
                    seqv=3;
                }
                break;
            case SEQV_RUN_BACKWARD: // run backward
                if (BTN_STOP||BTN_FORWARD)
                {
                    motor_stop(&motor);
                    main_timer = BUTTON_RELEASE_TIMEOUT;
                    seqv=3;
                }
                break;
            case SEQV_WAIT_BTN_RELEASE: // wait buttons released (min untill timeout)
                if ((!BTN_STOP)&&(!BTN_FORWARD)&&(!BTN_BACKWARD)&&(main_timer==0))
                {
                    main_timer = SLEEP_TIMEOUT;
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

	        if (main_timer) main_timer--;

	        motor_move(&motor);
	    }

		__bis_SR_register(CPUOFF + GIE); // enter sleep mode (leave on timer interrupt)
	}

	return -1;
}
