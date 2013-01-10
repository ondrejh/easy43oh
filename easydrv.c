/**
 * easydrv.c
 *
 *  Created on: 9.1.2013
 *      Author: O. Hejda
 *
 *  Description: EasyDriver (http://www.schmalzhaus.com/EasyDriver/index.html) module
 *  	main function periodically called handles motor outputs (turns motor)
 *      other supporting functions to set motor drive properties (speed, accel, etc..)
 *  	have fun!
 **/

/// includes
#include <msp430g2553.h>

#include "easydrv.h"

/// init motor data structure
void motor_init(t_motor *motor)
{
    motor->sleep=true;
    motor->speed=0;
    motor->position=0;

    motor->step_cnt=0;
    motor->sleeping=true;
}

/// set motor speed
void motor_run(t_motor *motor, int16_t speed)
{
    motor->sleep = false;
    motor->speed = speed;
}

/// stop motor
void motor_stop(t_motor *motor)
{
    motor->speed = 0;
}

/// release motor (set driver sleep mode)
void motor_sleep(t_motor *motor)
{
    motor->sleep = true;
}

/// periodically called motor function (to connect to some timer)
void motor_move(t_motor *motor)
{
    if (motor->sleep)
    {
        if (!motor->sleeping)
        {
            MOTOR_SLEEP_SET();
            MOTOR_LED_OFF();

            motor->speed=0;
            motor->position=0;
            motor->step_cnt=0;

            motor->sleeping=true;
        }
    }
    else
    {
        if (motor->sleeping)
        {
            MOTOR_SLEEP_RES();
            MOTOR_LED_ON();

            motor->sleeping=false;
        }
        else
        {
            // move motor (according to speed variable)
            motor->step_cnt+=motor->speed;
            if (motor->step_cnt>=SPEED_MAX)
            {
                // step forward
                MOTOR_DIR_FWD();
                motor->step_cnt-=SPEED_MAX;
                motor->position++;
                MOTOR_STEP();
            }
            else if (motor->step_cnt<=-SPEED_MAX)
            {
                // step back
                MOTOR_DIR_BCK();
                motor->step_cnt+=SPEED_MAX;
                motor->position--;
                MOTOR_STEP();
            }
        }
    }
}

