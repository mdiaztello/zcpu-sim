

#include "debug.h"
#include "memory_bus.h"
#include "memory_map.h"
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

    printf("the bus is writing to address %08x\n", bus->address_lines);

    //decode memory map
    if(bus->address_lines <= BOOT_ROM_END)
    {
        //NOTE: haven't implemented a BOOT ROM yet, so leave this commented out for now
        //bus->selected_device = BOOT_ROM_SELECTED;

        //FIXME: until we create an actual boot rom, we'll just say that we are
        //reading these addresses from RAM because that's where our little test
        //programs for the instruction set are being loaded in RAM starting at
        //address 0 I will eventually need to fix this by making a dummy boot
        //rom that loads the PC with the first address in RAM, and then using
        //the new PC as an index into our program array, but I haven't even
        //implemented a jump instruction yet!
        bus->selected_device = MEMORY_SELECTED;
    }
    else if((INTERRUPT_VECTOR_TABLE_START <= bus->address_lines) && (bus->address_lines <= INTERRUPT_VECTOR_TABLE_END))
    {
        bus->selected_device = MEMORY_SELECTED;
    }
    else if((GRAPHICS_REGION_START <= bus->address_lines) && (bus->address_lines <= GRAPHICS_REGION_END))
    {
        bus->selected_device = GRAPHICS_SELECTED;
    }
    else if((KEYBOARD_REGION_START <= bus->address_lines) && (bus->address_lines <= KEYBOARD_REGION_END))
    {
        bus->selected_device = KEYBOARD_SELECTED;
    }
    else //no special addresses, so pick normal memory
    {
        bus->selected_device = MEMORY_SELECTED;
    }
}
