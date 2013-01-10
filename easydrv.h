/**
 * easydrv.h
 *  Created on: 9.1.2013
 *      Author: O. Hejda
 *
 *  Description: EasyDriver (http://www.schmalzhaus.com/EasyDriver/index.html) module header
 *
 **/

#ifndef __EASYDRV_H__
#define __EASYDRV_H__

#include <inttypes.h>
#include <stdbool.h>

// motor output pins
#define MOTOR_SLEEP_PIN 2
#define MOTOR_DIR_PIN 1
#define MOTOR_STEP_PIN 0
// motor defines
#define MOTOR_INIT() {P2DIR|=0x07;P2OUT&=~0x07;}
#define MOTOR_SLEEP_SET() {P2OUT&=~(1<<MOTOR_SLEEP_PIN);}
#define MOTOR_SLEEP_RES() {P2OUT|=(1<<MOTOR_SLEEP_PIN);}
#define MOTOR_DIR_BCK() {P2OUT|=(1<<MOTOR_DIR_PIN);}
#define MOTOR_DIR_FWD() {P2OUT&=~(1<<MOTOR_DIR_PIN);}
#define MOTOR_STEP() {P2OUT|=(1<<MOTOR_STEP_PIN);P2OUT&=~(1<<MOTOR_STEP_PIN);}

#define MOTOR_LED_ON() {P1OUT|=0x40;}
#define MOTOR_LED_OFF() {P1OUT&=~0x40;}
#define MOTOR_LED_SWP() {P1OUT^=0x40;}


// maximum speed
#define SPEED_MAX 0x4000

/// motor data structure
typedef struct
{
    // drive variables
    bool sleep;
    int16_t speed;
    int32_t position;

    // goto position variables
    bool gotoen;
    int32_t gotopos;

    // internal variables
    int16_t step_cnt;
    bool sleeping;
} t_motor;

/// motor context initialization and position reset
void motor_init(t_motor *motor);
void motor_reset(t_motor *motor);

/// motor goto function (start and done)
void motor_goto(t_motor *motor, int32_t position, uint16_t speed);
bool motor_atposition(t_motor *motor);

/// basic functions (run, stop, sleep)
void motor_run(t_motor *motor, int16_t speed);
void motor_stop(t_motor *motor);
void motor_sleep(t_motor *motor);

/// main polling funtion (should be called periodically)
void motor_move(t_motor *motor);

#endif
