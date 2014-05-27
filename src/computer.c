
#include "computer.h"
#include "cpu.h"
//#include "memory_bus.h"
#include "memory.h"
//#include "graphics.h"
//#include "io.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>




struct computer_t 
{
    cpu_t* cpu;
    //memory_bus_t memory_bus;
    memory_t* RAM;
    //graphics_t screen;
};


//FIXME: these will need parameters for graphics and memory_bus later
//Creates a computer object and connects its dependencies, but doesn't 
//initialize it
computer_t* make_computer(cpu_t* cpu, memory_t* memory)
{
    computer_t* computer = calloc(1, sizeof(struct computer_t));
    computer->cpu = cpu;
    computer->RAM = memory;
    return computer;
}

//This is our CPU "factory" function, which handles the initialization and dependency
//injection to the CPU "constructor" separately
cpu_t* build_cpu(void)
{
    cpu_t* cpu = make_cpu();
    init_cpu(cpu);

    return cpu;
}

//creates a new computer system complete with all subsystems
//and initializes/resets it
computer_t* build_computer(void)
{
    const uint32_t NUM_MEM_LOCATIONS = 1024;

    cpu_t* cpu = build_cpu();
    memory_t* RAM = make_memory(NUM_MEM_LOCATIONS);
    computer_t* computer = make_computer(cpu, RAM);
    computer_reset(computer);
    return computer;
}

void computer_reset(computer_t* computer)
{
    cpu_reset(computer->cpu);
    memory_reset(computer->RAM);
    //reset_IO(computer->IO);
    //reset_memory_bus(computer->memory_bus);
    //reset_graphics(computer->screen);
}

//load the supplied program into computer memory
void computer_load_program(computer_t* computer, uint32_t* program, size_t program_length)
{
    for(size_t i = 0; i < program_length; i++)
    {
        memory_set(computer->RAM, i, program[i]);
    }
}

//execute the next single instruction for the program in memory
void computer_single_step(computer_t* computer)
{
    cpu_cycle(computer->cpu);
}

//execute the program in memory until told to stop
void computer_run(computer_t* computer)
{
    while(true)
    {
        cpu_cycle(computer->cpu);
    }
}

#if 0
void dump_memory(computer_t* computer, size_t starting_address, size_t ending_address)
{
    memory_print(computer->RAM, starting_address, ending_address);
}
#endif

