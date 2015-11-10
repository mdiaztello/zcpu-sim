
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "timer.h"
#include "bit_twiddling.h"

#include "debug.h"

#define Hz (1u)
#define MHz (1000000*Hz)
#define CPU_FREQUENCY (25*MHz)


enum timer_control_bits_t
{
    TIMER_ON_BIT = 0,
    TIMER_INTERRUPT_ENABLE_BIT = 1,
    TIMER_INTERRUPT_FLAG_BIT = 2,
};

const uint32_t MIN_PRESCALE_VALUE = 2;


//The timers on this machine can only receive their clock signal from the
//system clock, but they can divide down their own clocks by an arbitrary
//amount ranging between a factor of 2 and a factor of 2^32-1
//
//They have only an overflow interrupt in order to keep things simple. I might
//consider changing things in the future depending on how this scheme works
//out.

struct timer_t
{
    uint8_t control_bits;

    uint8_t IRQ_number;
    uint32_t prescale_value;
    uint32_t prescale_counter;

    uint32_t timer_value;
};


static void prescale_tick(timer_t* timer);
static void tick(timer_t* timer);
static void update_interrupt_status(timer_t* timer, interrupt_controller_t* ic);
static void update_timer_overflow_status(timer_t* timer);


timer_t* make_timer(uint8_t IRQ_number)
{
    //timer has value of 0, no prescaling, is turned off, and has its
    //interrupts masked when first created
    timer_t* timer =  calloc(1, sizeof(struct timer_t));
    timer->IRQ_number = IRQ_number;

    //DEBUG

    BIT_SET(timer->control_bits, TIMER_ON_BIT);
    BIT_SET(timer->control_bits, TIMER_INTERRUPT_ENABLE_BIT);
    timer->prescale_value = 0;
    timer->timer_value = UINT32_MAX - 10*CPU_FREQUENCY;
    //timer->timer_value = 0;

    return timer;
}


void timer_cycle(timer_t* timer, memory_bus_t* bus, interrupt_controller_t* ic)
{
    //timer register manipulation here

        //FIXME: figure out what I want to do with the timer-specific
        //registers: should I decode them "by hand" or just stick everything in
        //a lookup table?

    if(CHECK_BIT_CLEAR(timer->control_bits, TIMER_ON_BIT))
        return;

    uint32_t previous_timer_value = timer->timer_value;

    tick(timer);

    if(timer->timer_value < previous_timer_value)
    {
        beacon();
        update_timer_overflow_status(timer);
        update_interrupt_status(timer, ic);
    }

}

static void update_interrupt_status(timer_t* timer, interrupt_controller_t* ic)
{
    if(CHECK_BIT_SET(timer->control_bits, TIMER_INTERRUPT_ENABLE_BIT) && CHECK_BIT_SET(timer->control_bits, TIMER_INTERRUPT_FLAG_BIT))
    {
        //at this point, interrupts are enabled and the overflow interrupt has
        //occured, so we need to signal the processor
        request_interrupt(ic, timer->IRQ_number);
    }
}


static void prescale_tick(timer_t* timer)
{
    if(timer->prescale_counter < timer->prescale_value)
    {
        timer->prescale_counter++;
    }
    else
    {
        //counting up from 1 fixes the off-by-one error and allows us to think
        //in terms of the prescaler factor instead of (prescaler factor - 1)
        timer->prescale_counter = 1;
        timer->timer_value++;
    }
}

static bool prescaling_enabled(timer_t* timer)
{
    return (MIN_PRESCALE_VALUE <= timer->prescale_value);
}

static void tick(timer_t* timer)
{
    if(!prescaling_enabled(timer))
    {
        timer->timer_value++;
    }
    else
    {
        prescale_tick(timer);
    }
}

// This function sets the timer overflow flag. It is only allowed to set this
// flag (rather than clear it, too) because code running in the foreground on
// the processor may want to poll the flag to see when the timer overflows
// (rather than using a timer interrupt).
static void update_timer_overflow_status(timer_t* timer)
{
    BIT_SET(timer->control_bits, TIMER_INTERRUPT_FLAG_BIT);
}

