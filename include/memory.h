

//This module contains the memory for the system and handles the memory mapping for the I/O

#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct memory memory_t;


memory_t* make_memory(size_t mem_size);
void memory_reset(memory_t* RAM);

uint32_t memory_get(memory_t* RAM, size_t address);
void memory_set(memory_t* RAM, size_t address, uint32_t value);
void memory_print(memory_t* RAM, size_t starting_address, size_t ending_address);
void memory_cycle(memory_t* RAM, memory_bus_t* bus);



#endif
