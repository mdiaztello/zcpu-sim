
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "interrupt_controller.h"


#define MAX_NUM_IRQS    256

struct interrupt_controller_t
{
    uint8_t interrupt_source;
    bool interrupt_requested;
    bool irq_sources[MAX_NUM_IRQS];
};


void update_interrupt_request_status(interrupt_controller_t* ic)
{
    bool interrupt_source_found = false;
    ic->interrupt_requested = false;

    for(int i = 0; i < MAX_NUM_IRQS; i++)
    {
        if(!interrupt_source_found && ic->irq_sources[i])
        {
            interrupt_source_found = true;
            ic->interrupt_source = i;
        }
        ic->interrupt_requested = ic->interrupt_requested || ic->irq_sources[i];
    }
}

void request_interrupt(interrupt_controller_t* ic, uint8_t irq_number)
{
    ic->irq_sources[irq_number] = true;
    update_interrupt_request_status(ic);
}

void clear_interrupt(interrupt_controller_t* ic, uint8_t irq_number)
{
    ic->irq_sources[irq_number] = false;
    update_interrupt_request_status(ic);
}
