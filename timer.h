/*
 * timer.h
 *
 *  Created on: 22.8.2012
 *      Author: ohejda
 *
 *  Description: timer module using interrupts
 *
 *  Functions:
 *  	timer_init(void) .. timer initialization
 *
 *  Interrupt routines:
 *  	Timer A0 interrupt service routine .. set new timeout and exit sleep mode
 *
 */

#ifndef __TIMER_H__
#define __TIMER_H__

// timer interval (1ms / 1MHz osc / fosc/8)
#define TIMER_INTERVAL 125

void timer_init(void);

#endif
