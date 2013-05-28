
#include "memory.h"
#include <stdlib.h>


//I'm not sure if this memory structure needs to be more elaborate. Just leave it alone for now
struct memory
{
    uint32_t* system_memory;
}; 

memory_t* make_memory(size_t mem_size)
{
    memory_t* RAM = calloc(1, sizeof(struct memory));
    RAM->system_memory = calloc(mem_size, sizeof(uint32_t));
    return RAM;
}

uint32_t memory_get(memory_t* RAM, size_t address)
{
    //FIXME: memory mapped IO stuff here as well
    return RAM->system_memory[address];
}

void memory_set(memory_t* RAM, size_t address, uint32_t value)
{
    //FIXME: memory mapped IO stuff here as well
    RAM->system_memory[address] = value;
}
