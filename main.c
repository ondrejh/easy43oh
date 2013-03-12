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
//            |             P2.3| <-- BUTTON STOP      (all connecting gnd)
//            |             P2.4| <-- BUTTON FORWARD
//            |             P2.5| <-- BUTTON BACKWARD
//            |                 |
//            |             P2.6| <-- FRONT END SWITCH (both connecting gnd)
//            |             P2.7| <-- REAR END SWITCH
//            |                 |
//            |       P1.4(ADC4)| <-- V supply
//            |       P1.5(ADC5)| <-- V output
//            |                 |

//******************************************************************************

#define DEBUG

// include section
#include <msp430g2553.h>
#include <inttypes.h>
//#include <stdbool.h>
#include "timer.h"
#include "uart.h"
#include "easydrv.h"
#include "adc.h"

// board (leds, button)
#define LED_INIT() {P1DIR|=0x41;P1OUT&=~0x41;}
#define LED_RED_ON() {P1OUT|=0x01;}
#define LED_RED_OFF() {P1OUT&=~0x01;}
#define LED_RED_SWAP() {P1OUT^=0x01;}
#define LED_GREEN_ON() {P1OUT|=0x40;}
#define LED_GREEN_OFF() {P1OUT&=~0x40;}
#define LED_GREEN_SWAP() {P1OUT^=0x40;}

// button pins
#define BTN_STOP_PIN     3
#define BTN_FORWARD_PIN  4
#define BTN_BACKWARD_PIN 5
// buttons init
#define BUTTONS_INIT() {P2DIR&=~((1<<BTN_STOP_PIN)|(1<<BTN_FORWARD_PIN)|(1<<BTN_BACKWARD_PIN));\
                        P2OUT|=((1<<BTN_STOP_PIN)|(1<<BTN_FORWARD_PIN)|(1<<BTN_BACKWARD_PIN));\
                        P2REN|=((1<<BTN_STOP_PIN)|(1<<BTN_FORWARD_PIN)|(1<<BTN_BACKWARD_PIN));}
// button input functions
#define BTN_STOP     ((P2IN&(1<<BTN_STOP_PIN))==0)
#define BTN_FORWARD  ((P2IN&(1<<BTN_FORWARD_PIN))==0)
#define BTN_BACKWARD ((P2IN&(1<<BTN_BACKWARD_PIN))==0)

// end switch
#define ENDSW_FRONT_PIN 6
#define ENDSW_REAR_PIN  7
// end switch initialization
#define ENDSW_INIT() {P2DIR&=~((1<<ENDSW_FRONT_PIN)|(1<<ENDSW_REAR_PIN));\
                      P2SEL&=~((1<<ENDSW_FRONT_PIN)|(1<<ENDSW_REAR_PIN));\
                      P2OUT|=((1<<ENDSW_FRONT_PIN)|(1<<ENDSW_REAR_PIN));\
                      P2REN|=((1<<ENDSW_FRONT_PIN)|(1<<ENDSW_REAR_PIN));}
// end switch input funcitons
#define ENDSW_FRONT ((P2IN&(1<<ENDSW_FRONT_PIN))==0)
#define ENDSW_REAR  ((P2IN&(1<<ENDSW_REAR_PIN))==0)

// timeout for motor goto sleep mode [ticks]
#define SLEEP_TIMEOUT 20000 // 2s
#define BUTTON_RELEASE_TIMEOUT 1000 // 100ms

// sequentinal state definitions
#define SEQV_SLEEP            -1
#define SEQV_WAIT_BTN          0
#define SEQV_RUN_FORWARD       1
#define SEQV_RUN_BACKWARD      2
#define SEQV_WAIT_BTN_RELEASE  3

// analog input status
#define ANIN_LOW -1
#define ANIN_OPEN 0
#define ANIN_HIGH 1

// hw depended init
void board_init(void)
{
	// oscillator
	BCSCTL1 = CALBC1_8MHZ; // Set DCO
	DCOCTL  = CALDCO_8MHZ;

    MOTOR_INIT();   // motor outputs
	LED_INIT();     // leds
	BUTTONS_INIT(); // buttons
	ENDSW_INIT();   // end switch
}

// test if output is clamped high or low
int8_t evaluate_analog_input(void)
{
    int8_t last_eval = 0;

    uint16_t Vpos = get_adc(0);
    uint16_t Vout = get_adc(1);

    uint16_t THold = Vpos/4;
    uint16_t Hyst = 5;

    switch (last_eval)
    {
        case ANIN_LOW:
            if (Vout>(THold+Hyst))
            {
                if (Vout>(Vpos-THold+Hyst))
                    last_eval = ANIN_HIGH;
                else
                    last_eval = ANIN_OPEN;
            }
            break;
        case ANIN_OPEN:
            if (Vout<(THold-Hyst))
                last_eval = ANIN_LOW;
            else if (Vout>(Vpos-THold+Hyst))
                last_eval = ANIN_HIGH;
            break;
        case ANIN_HIGH:
            if (Vout<(Vpos-THold-Hyst))
            {
                if (Vout<(THold-Hyst))
                    last_eval = ANIN_LOW;
                else
                    last_eval = ANIN_OPEN;
            }
            break;
    }

    return last_eval;
}

// main program body
int main(void)
{
    int8_t analog_eval = 0;
    int16_t seqv = -1;
    uint16_t main_timer = 0;

    int8_t analog_eval_onstart = 0;

    t_motor motor; // motor context


	WDTCTL = WDTPW + WDTHOLD;	// Stop WDT


	board_init();       // init oscilator and leds
	timer_init();       // init timer
	uart_init();        // init uart interface
	motor_init(&motor); // init motor context
	init_adc();         // init adc
	start_adc();        // start first conversion

    // main loop
	while(1)
	{
	    // read and evaluate adc input
	    if (adc_ready())
	    {
	        analog_eval = evaluate_analog_input();
            if (analog_eval==0) {LED_RED_OFF();} else {LED_RED_ON();}
	        set_adc_val(0,get_adc(0));
	        set_adc_val(1,get_adc(1));
	        start_adc();
        }

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
                if (BTN_FORWARD && (!ENDSW_FRONT))
                {
                    analog_eval_onstart = analog_eval;
                    motor_run(&motor,SPEED_MAX/4);
                    seqv=1;
                }
                else if (BTN_BACKWARD && (!ENDSW_REAR))
                {
                    analog_eval_onstart = analog_eval;
                    motor_run(&motor,-SPEED_MAX/4);
                    seqv=2;
                }
                else if (main_timer==0)
                {
                    motor_sleep(&motor);
                    seqv--;
                }
                break;
            case SEQV_RUN_FORWARD: // run forward
                if (BTN_STOP||BTN_BACKWARD||ENDSW_FRONT||(analog_eval!=analog_eval_onstart))
                {
                    motor_stop(&motor);
                    main_timer = BUTTON_RELEASE_TIMEOUT;
                    seqv=3;
                }
                break;
            case SEQV_RUN_BACKWARD: // run backward
                if (BTN_STOP||BTN_FORWARD||ENDSW_REAR||(analog_eval!=analog_eval_onstart))
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
