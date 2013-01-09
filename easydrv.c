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

t_motor motor;


/// init motor data structure
void init_motor(t_motor *motor)
{
    motor->sleep=true;
    motor->speed=0;
    motor->position=0;

    motor->step_cnt=0;
}

/// periodically called motor function (to connect to some timer)
void move_motor(t_motor *motor)
{
    if (motor->sleep)
    {
        MOTOR_SLEEP_SET();
        MOTOR_LED_OFF();

        motor->speed=0;
        motor->position=0;
        motor->step_cnt=0;
    }
    else
    {
        MOTOR_SLEEP_RES();
        MOTOR_LED_ON();

        // move motor (according to speed variable)
        motor->step_cnt+=motor->speed;
        if (motor->step_cnt>=SPEED_MAX)
        {
            MOTOR_DIR_FWD();
            motor->step_cnt-=SPEED_MAX;
            MOTOR_STEP();
        }
        else if (motor->step_cnt<=-SPEED_MAX)
        {
            MOTOR_DIR_BCK();
            motor->step_cnt+=SPEED_MAX;
            MOTOR_STEP();
        }
    }
}
