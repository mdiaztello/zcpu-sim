

#ifndef __TIMER_H_
#define __TIMER_H_

#include "memory_bus.h"
#include "interrupt_controller.h"

typedef struct timer_t timer_t;

timer_t* make_timer(uint8_t IRQ_number);
void timer_cycle(timer_t* timer, memory_bus_t* bus, interrupt_controller_t* ic);

#endif
