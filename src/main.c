


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "debug.h"

#include "computer.h"

//This will allow us to write test programs inline without manually encoding
//the instructions. The only caveat is that there is no support for labels, so
//jumps have to be calculated and placed manually.
#include "preprocessor_assembler.h"




static void quit_simulation(void)
{
    //clean_up();
    exit(EXIT_SUCCESS);
}


#if 0
void input(void)
{
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
        if(e.type == SDL_QUIT)
        {
            simulation_running = false;
            break;
        }
        else if(e.type == SDL_KEYDOWN)
        {
            if(e.key.keysym.sym == SDLK_q)
            {
                simulation_running = false;
                break;
            }
            else
            {
                printf("Some other key was pressed...\n");
            }
        }
    }
}
#endif




#define PROGRAM_LENGTH 0x100
//uint32_t program[PROGRAM_LENGTH] = { 0x0400AABB, 0x0801AABB,0x1002AABB, 0x2003AABB };
//uint32_t program[PROGRAM_LENGTH] = {0x08000000,0x08000000,0x08000000}; //negate r0 3 times
//uint32_t program[PROGRAM_LENGTH] = {0x08000000,0x08000000,0x08200000}; //negate r0 3 times and stick it in r1 on the last negation
//uint32_t program[PROGRAM_LENGTH] = {0x0400FFFF,0x04215A5B,0x04000000}; // OR R0 R0 #7FFF, OR R1 R1 #2D2D, OR R0 R0 R0

//uint32_t program[PROGRAM_LENGTH] = { 0x0400AABB, 0x0801AABB,0x1002AABB, 0x2003AABB };
//uint32_t program[PROGRAM_LENGTH] = {NOT(R0, R0), NOT(R0, R0), NOT(R0, R0)}; //check that negating 3 times is the same as negating once
//uint32_t program[PROGRAM_LENGTH] = {NOT(R0, R0), NOT(R0, R0), NOT(R0, R1)}; //check that negating 3 times is the same as negating once, but change the output of the final negation
uint32_t program[PROGRAM_LENGTH] = {OR_IMMEDIATE(R0, R0, 0x7FFF), OR_IMMEDIATE(R1, R1, 0x2D2D), OR(R0, R0, R0)}; // check immediate mode OR on different registers, check that ORing with self changes nothing





int main(void)
{
    computer_t* computer = build_computer();
    computer_load_program(computer, program, PROGRAM_LENGTH);

#if 0
    computer_single_step(computer);
    computer_print_elapsed_cycles(computer);
    computer_single_step(computer);
    computer_print_elapsed_cycles(computer);
    computer_single_step(computer);
    computer_print_elapsed_cycles(computer);
    computer_single_step(computer);
    computer_print_elapsed_cycles(computer);

    dump_computer_cpu_state(computer);
    dump_computer_memory(computer, 0x00, 0x10);
#endif

    computer_run(computer);

    quit_simulation();

    return 0;
}
