

#ifndef __TIMER_H_
#define __TIMER_H_

#include "memory_bus.h"

typedef struct timer_t timer_t;

timer_t* make_timer(void);
void timer_cycle(timer_t* timer, memory_bus_t* bus);

#endif
