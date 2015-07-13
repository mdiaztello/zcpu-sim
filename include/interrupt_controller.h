


#ifndef __INTERRUPT_CONTROLLER_H_
#define __INTERRUPT_CONTROLLER_H_


typedef struct interrupt_controller_t interrupt_controller_t;

interrupt_controller_t* make_interrupt_controller(void);
void request_interrupt(interrupt_controller_t* ic, uint8_t irq_number);

#endif
