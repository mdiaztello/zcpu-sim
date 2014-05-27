


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "debug.h"

#include "computer.h"
//#include "cpu.h"
//#include "graphics.h"
//#include "keyboard.h"

// include any SDL stuff here


#define PROGRAM_LENGTH 0x100
//uint32_t program[PROGRAM_LENGTH] = { 0x0400AABB, 0x0801AABB,0x1002AABB, 0x2003AABB };
//uint32_t program[PROGRAM_LENGTH] = {0x08000000,0x08000000,0x08000000}; //negate r0 3 times
//uint32_t program[PROGRAM_LENGTH] = {0x08000000,0x08000000,0x08200000}; //negate r0 3 times and stick it in r1 on the last negation
uint32_t program[PROGRAM_LENGTH] = {0x0400FFFF,0x04215A5B,0x04000000}; // OR R0 R0 #7FFF, OR R1 R1 #2D2D, OR R0 R0 R0




int main(void)
{
    //cpu_t* cpu = make_cpu();
    //init_cpu(cpu);
    crashprint();
    computer_t* computer = build_computer();
    crashprint();
    
    computer_load_program(computer, program, PROGRAM_LENGTH);
    crashprint();

    //cpu_cycle(cpu);
    //cpu_cycle(cpu);
    //cpu_cycle(cpu);
    //cpu_cycle(cpu);
    
    crashprint();
#if 0
    computer_single_step(computer);
    computer_single_step(computer);
    computer_single_step(computer);
    computer_single_step(computer);
#endif
    crashprint();

    //dump_cpu_state(computer->cpu);
    //dump_memory(computer->RAM, 0x00, 0x10);

    return 0;
}
