
// ----------------------------------------------------------------------------
//
//  FILE: memory.c
//
//  DESCRIPTION: This module implements our computer's RAM and is kept separate
//  from the CPU module partly to reflect that memory in personal computer
//  systems is located on separate chips and partly to decouple the
//  implementation of the two modules. Decoupling things in this way means that
//  changing the implementation of the RAM module (to do things like simulating
//  large access times or banked access) can be done easily and the scope of
//  the changes can be limited.
//
// ----------------------------------------------------------------------------

// TODO: I may want to simulate cached memory access in the future, so I will
// eventually need to change the delay time to access this memory module.

#include "debug.h"
#include "memory_bus.h"
#include "memory.h"
#include <stdlib.h>
#include <string.h>


//I'm not sure if this memory structure needs to be more elaborate. Just leave it alone for now
struct memory
{
    uint32_t memory_size;
    uint32_t* system_memory;
}; 

memory_t* make_memory(size_t mem_size)
{
    memory_t* RAM = calloc(1, sizeof(struct memory));
    RAM->memory_size = mem_size;
    RAM->system_memory = calloc(RAM->memory_size, sizeof(uint32_t));
    return RAM;
}

void memory_reset(memory_t* RAM)
{
    memset(RAM->system_memory, 0x00, RAM->memory_size);
}

uint32_t memory_get(memory_t* RAM, size_t address)
{
    return RAM->system_memory[address];
}

void memory_set(memory_t* RAM, size_t address, uint32_t value)
{
    RAM->system_memory[address] = value;
}

//prints the range in memory from the starting to the ending address inclusive
void memory_print(memory_t* RAM, size_t starting_address, size_t ending_address)
{
    printf("\n----- DUMPING CONTENTS OF RAM -----\n");
    for(size_t i = starting_address; i <= ending_address; i++)
    {
        printf("address = 0x%08X \t contents = 0x%08X\n", (unsigned int)i, memory_get(RAM,i));
    }
    printf("\n");

}


void memory_cycle(memory_t* RAM, memory_bus_t* bus)
{
    if(MEMORY_SELECTED != bus_get_selected_device(bus) || !bus_is_enabled(bus))
    {
        return;
    }
    
    //for now, memory will take at least one cycle to read/write
    //static const uint32_t MAX_CYCLES = 80000;
    static const uint32_t MAX_CYCLES = 1;
    static uint32_t cycle_count = 0;

    if(cycle_count < MAX_CYCLES)
    {
        cycle_count++;
        return;
    }
    else
    {
        cycle_count = 0;
        if(bus_is_write_operation(bus))
        {
            memory_set(RAM, bus_get_address_lines(bus), bus_get_data_lines(bus));
        }
        else
        {
            bus_set_data_lines(bus, memory_get(RAM, bus_get_address_lines(bus)));
        }
        bus_set_device_ready(bus); //read/write complete
    }
}
