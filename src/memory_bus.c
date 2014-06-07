

#include "debug.h"
#include "memory_bus.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


struct memory_bus_t
{
    uint32_t address_lines;
    uint32_t data_lines;
    bus_mode_t bus_mode; //for either read or write ops
    selected_device_t selected_device;
    bool device_ready; //report back the status of devices that take multiple bus cycles
    bool bus_active;
};

memory_bus_t* make_memory_bus(void)
{
    memory_bus_t* bus = calloc(1, sizeof(struct memory_bus_t));
    return bus;
}

//the device being read/written to is signaling the data is ready
void bus_set_device_ready(memory_bus_t* bus)
{
    bus->device_ready = true;
}

//the cpu must clear the ready signal after its read the data
//so we don't get confused
void bus_clear_device_ready(memory_bus_t* bus)
{
    bus->device_ready = true;
}

bool bus_is_device_ready(memory_bus_t* bus)
{
    return bus->device_ready;
}

void bus_enable(memory_bus_t* bus)
{
    bus->bus_active = true;
}

void bus_disable(memory_bus_t* bus)
{
    bus->bus_active = false;
}

bool bus_is_enabled(memory_bus_t* bus)
{
    return bus->bus_active;
}

void bus_set_read_operation(memory_bus_t* bus)
{
    bus->bus_mode = DATA_READ;
}

void bus_set_write_operation(memory_bus_t* bus)
{
    bus->bus_mode = DATA_WRITE;
}

bool bus_is_write_operation(memory_bus_t* bus)
{
    return bus->bus_mode == DATA_WRITE;
}

bool bus_is_read_operation(memory_bus_t* bus)
{
    return bus->bus_mode == DATA_READ;
}

void bus_set_address_lines(memory_bus_t* bus, uint32_t addr)
{
    bus->address_lines = addr;
}

void bus_set_data_lines(memory_bus_t* bus, uint32_t data)
{
    bus->data_lines = data;
}

uint32_t bus_get_address_lines(memory_bus_t* bus)
{
    return bus->address_lines;
}

uint32_t bus_get_data_lines(memory_bus_t* bus)
{
    return bus->data_lines;
}

selected_device_t bus_get_selected_device(memory_bus_t* bus)
{
    return bus->selected_device;
}

void bus_cycle(memory_bus_t* bus)
{
    //if the bus is inactive, don't process anything
    if(!bus_is_enabled(bus))
    {
        bus->selected_device = NO_DEVICE_SELECTED;
        bus->device_ready = false;
        return;
    }

    //decode memory map
    if(false)
    {
        crashprint();
    }
    else //no special addresses, so pick normal memory
    {
        bus->selected_device = MEMORY_SELECTED;
    }
}
