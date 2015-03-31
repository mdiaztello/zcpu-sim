


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
    exit(EXIT_SUCCESS);
}


#define PROGRAM_LENGTH 0x100
//uint32_t program[PROGRAM_LENGTH] = { 0x0400AABB, 0x0801AABB,0x1002AABB, 0x2003AABB };
//uint32_t program[PROGRAM_LENGTH] = {0x08000000,0x08000000,0x08000000}; //negate r0 3 times
//uint32_t program[PROGRAM_LENGTH] = {0x08000000,0x08000000,0x08200000}; //negate r0 3 times and stick it in r1 on the last negation
//uint32_t program[PROGRAM_LENGTH] = {0x0400FFFF,0x04215A5B,0x04000000}; // OR R0 R0 #7FFF, OR R1 R1 #2D2D, OR R0 R0 R0

//uint32_t program[PROGRAM_LENGTH] = { 0x0400AABB, 0x0801AABB,0x1002AABB, 0x2003AABB };
//uint32_t program[PROGRAM_LENGTH] = {NOT(R0, R0), NOT(R0, R0), NOT(R0, R0)}; //check that negating 3 times is the same as negating once
//uint32_t program[PROGRAM_LENGTH] = {NOT(R0, R0), NOT(R0, R0), NOT(R0, R1)}; //check that negating 3 times is the same as negating once, but change the output of the final negation

#if 0
// check immediate mode OR on different registers, check that ORing with self changes nothing
uint32_t program[PROGRAM_LENGTH] = {
    OR_IMMEDIATE(R0, R0, 0x7FFF), 
    OR_IMMEDIATE(R1, R1, 0x2D2D), 
    OR(R0, R0, R0)
};
#endif

#if 0
//check the load instructions
uint32_t program[PROGRAM_LENGTH] = {
    LOAD(R0, 0xFFFFFFFF),
    LOAD(R1, 0x00000000),
    LOAD(R2, -2),
};
#endif

#if 0
//check the store instructions
uint32_t program[PROGRAM_LENGTH] = {
    OR_IMMEDIATE(R0, R0, 0xABC),
    STORE(R0, 10),
    STORE(R0, -1),
};
#endif

#if 0
//check the LOADA instructions
uint32_t program[PROGRAM_LENGTH] = {
    LOADA(R0, 0x1100)
};
#endif

#if 0
//check writing to the framebuffer
//by writing a 6x6 blue square
#define FRAME_BUFFER_START (0x1100)
#define BLUE (0x0000FFFF)
uint32_t program[PROGRAM_LENGTH] = {
    LOADA(R1, FRAME_BUFFER_START),
    OR_IMMEDIATE(R2, R2, BLUE),
    STORER(R2, R1, (0*640+320)),
    STORER(R2, R1, (0*640+321)),
    STORER(R2, R1, (0*640+322)),
    STORER(R2, R1, (0*640+323)),
    STORER(R2, R1, (0*640+324)),
    STORER(R2, R1, (0*640+325)),
    STORER(R2, R1, (1*640+320)),
    STORER(R2, R1, (1*640+321)),
    STORER(R2, R1, (1*640+322)),
    STORER(R2, R1, (1*640+323)),
    STORER(R2, R1, (1*640+324)),
    STORER(R2, R1, (1*640+325)),
    STORER(R2, R1, (2*640+320)),
    STORER(R2, R1, (2*640+321)),
    STORER(R2, R1, (2*640+322)),
    STORER(R2, R1, (2*640+323)),
    STORER(R2, R1, (2*640+324)),
    STORER(R2, R1, (2*640+325)),
    STORER(R2, R1, (3*640+320)),
    STORER(R2, R1, (3*640+321)),
    STORER(R2, R1, (3*640+322)),
    STORER(R2, R1, (3*640+323)),
    STORER(R2, R1, (3*640+324)),
    STORER(R2, R1, (3*640+325)),
    STORER(R2, R1, (4*640+320)),
    STORER(R2, R1, (4*640+321)),
    STORER(R2, R1, (4*640+322)),
    STORER(R2, R1, (4*640+323)),
    STORER(R2, R1, (4*640+324)),
    STORER(R2, R1, (4*640+325)),
    STORER(R2, R1, (5*640+320)),
    STORER(R2, R1, (5*640+321)),
    STORER(R2, R1, (5*640+322)),
    STORER(R2, R1, (5*640+323)),
    STORER(R2, R1, (5*640+324)),
    STORER(R2, R1, (5*640+325)),
    HCF
};
#undef FRAME_BUFFER_START
#undef BLUE
#endif

#if 0

//FIXME: this little program is incomplete. We can't clear the framebuffer
//without first adding branch instructions and the add instruction
//clear the framebuffer to white
#define FRAME_BUFFER_START (0x1100 - 1)
uint32_t program[PROGRAM_LENGTH] = {
    LOADA(R1, FRAME_BUFFER_START),
    CLEAR(R2),
    NOT(R2, R2),
    STORER(R2, R1, (0*640+320)),
    HCF
}
#undef FRAME_BUFFER_START

#endif


//small program to test adding capability by itself
uint32_t program[PROGRAM_LENGTH] = {
    CLEAR(R29),
    ADD_IMMEDIATE(R29, R29, 1),
    ADD(R29, R29, R29),
    ADD(R29, R29, R29),
    ADD(R29, R29, R29),
    HCF
};

//throwaway function so I don't have to keep commenting/uncommenting the "run x steps" code
static void run(computer_t* computer, int num_steps)
{
    dump_computer_memory(computer, 0x00, 0x10);
    if(num_steps < 0)
    {
        computer_run(computer);
    }
    else
    {
        for(int i = 0; i< 20; i++)
        {
            computer_single_step(computer);
            computer_print_elapsed_cycles(computer);
            dump_computer_cpu_state(computer);
            printf("\n\n");
        }
    }
}

int main(void)
{
    computer_t* computer = build_computer();
    computer_load_program(computer, program, PROGRAM_LENGTH);

    const int RUN_FOREVER = -1;
    const int num_steps = 8;
    run(computer, num_steps);

    quit_simulation();

    return 0;
}
