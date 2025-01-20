/* 
 * File:   Tick.h
 * Author: Rob Dunford of Sidmouth Tech Ltd
 * 
 * Keeps a count of a number of ticks, incremented by a call from an external source such as a timer interrupt
 * Provides functions for comparing the current tick count to previous count values
 */

#ifndef TICK_H
#define TICK_H


#include <stdint.h>
#include <stdbool.h>

typedef uint32_t tick_t;

#define tick_t_MAX UINT32_MAX

//#define TickGetTicks() (tick_t)millis()
// /*
//  * Increments the current tick count
//  */
// void TickIncrement(void);

// /*
//  * Get the current tick count
//  *
//  * Return
//  *  -   current tick count
// */
tick_t TickGetTicks(void);  //get current tick count

/*
 * Get the number of elapsed ticks since a previous tick count value
 * 
 * Parameters
 *      previousTick    -   starting tick count
 * 
 * Return
 *      number of ticks since starting tick count
 */
tick_t TicksSince(tick_t previousTick);

bool TicksHavePassed(tick_t* tickVariable, tick_t duration);

#endif /* TICK_H */
