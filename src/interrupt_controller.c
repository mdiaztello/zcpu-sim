
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "interrupt_controller.h"
#include "queue.h"

// The interrupt controller is the hardware module that interacts with the
// peripheral devices that are capable of requesting interrupts from the CPU.
// Peripherals simulate pulling on an interrupt request line by calling
// request_interrupt() with their assigned IRQ number. The actual hardware
// would have individual input lines for each possible interrupt source and
// would convert the asserted interrupt line into the appropriate IRQ number in
// order to relay that info to the processor, which will use the IRQ number to
// determine which interrupt service routine to jump to.

//TODO: make sure to add some mechanism for globally disabling interrupts (or
//at least the maskable interrupts)

struct interrupt_controller_t
{
    // Each interrupt request in the queue contains the 8-bit interrupt vector
    // number of the device requesting the interrupt
    queue_t* interrupt_requests;
    uint32_t INTERRUPT_VECTOR_TABLE_START_ADDRESS;
};

interrupt_controller_t* make_interrupt_controller(uint32_t ivt_start_address)
{
    interrupt_controller_t* ic = calloc(1, sizeof(struct interrupt_controller_t));
    ic->interrupt_requests = queue_create(MAX_NUM_IRQS);
    ic->INTERRUPT_VECTOR_TABLE_START_ADDRESS = ivt_start_address;
    return ic;
}

void destroy_interrupt_controller(interrupt_controller_t* ic)
{
    free(ic->interrupt_requests);
    free(ic);
}

//External interface function that hardware peripherals call to make interrupt
//requests
void request_interrupt(interrupt_controller_t* ic, uint8_t irq_number)
{
    queue_put(ic->interrupt_requests, irq_number);
}

//Reports back if at least one peripheral device has requested an interrupt
bool interrupt_requested(interrupt_controller_t* ic)
{
    return !queue_is_empty(ic->interrupt_requests);
}

//Precondition: interrupt_requested() must be true in order for this function to work
uint8_t get_interrupt_source(interrupt_controller_t* ic)
{
    queue_return_data_t interrupt_request =  queue_get(ic->interrupt_requests);
    return interrupt_request.value;
}

uint32_t get_interrupt_vector_table_starting_address(interrupt_controller_t* ic)
{
    return ic->INTERRUPT_VECTOR_TABLE_START_ADDRESS;
}
