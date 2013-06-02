


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "cpu.h"
//#include "graphics.h"
//#include "keyboard.h"

// include any SDL stuff here


#define PROGRAM_LENGTH 0x100
//uint32_t program[PROGRAM_LENGTH] = { 0x0400AABB, 0x0801AABB,0x1002AABB, 0x2003AABB };
//uint32_t program[PROGRAM_LENGTH] = {0x08000000,0x08000000,0x08000000}; //negate r0 3 times
uint32_t program[PROGRAM_LENGTH] = {0x08000000,0x08000000,0x08200000}; //negate r0 3 times and stick it in r1 on the last negation




int main(void)
{

    cpu_t* cpu = make_cpu();
    init_cpu(cpu);
    cpu_load_program(cpu, program, PROGRAM_LENGTH);
    cpu_cycle(cpu);
    cpu_cycle(cpu);
    cpu_cycle(cpu);
    cpu_cycle(cpu);
    dump_cpu_state(cpu);
    dump_memory(cpu, 0x00, 0x10);


    return 0;
}
