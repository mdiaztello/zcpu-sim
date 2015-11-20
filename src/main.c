
// ----------------------------------------------------------------------------
//
//  FILE: main.c
//
//  DESCRIPTION: This file acts like a simple driver for the whole simulator. 
//  It will just build the computer, pass it a program to run, and then begin
//  the simulation.
//
// ----------------------------------------------------------------------------

// TODO: Instead of writing an ASM program inside of this file and using the
// preprocessor assembler, I think I will want to pass in a
// pre-compiled/assembled file that the simulated computer can load from.
// Eventually this can go from being a raw executable program to a file that
// represents a fake hard drive image, which would more accurately reflect how
// a real personal computer system boots.

// TODO: I don't know at the moment, but it may be better to separate out the
// graphics simulation code from the SDL initialization code and then stick the
// SDL initialization here so that the simulator can run without needing to
// create a window each time. I haven't decided how to do this just yet.

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

#define GET_ARRAY_LENGTH(array) ((sizeof(array)) / (sizeof(array[0])))

#define PROGRAM_LENGTH (GET_ARRAY_LENGTH(program) + 100)

#include "memory_map.h" //for the GRAPHICS_REGION_START definition

//check writing to the framebuffer with branching by writing a 6x6 blue square
#define FRAME_BUFFER_START (GRAPHICS_REGION_START)
#define BLUE (0x0000FFFF)
#define GREEN (0x00FF00FF)
#define RED (0xFF0000FF)
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define BOX_WIDTH   100
#define BOX_HEIGHT  100
#define STARTING_POSITION_IN_FRAME_BUFFER    (((SCREEN_HEIGHT/2) - (BOX_HEIGHT/2))*SCREEN_WIDTH + ((SCREEN_WIDTH/2) - (BOX_WIDTH/2)))
#define SOFTWARE_INTERRUPT_SOURCE   47

#define PIXEL_POSITION R1
#define PIXEL_VALUE R2
#define BOX_WIDTH_COUNTER R4
#define BOX_HEIGHT_COUNTER R5
#define INC(x) ((ADD_IMMEDIATE((x),(x),1)))
#define DEC(x) ((ADD_IMMEDIATE((x),(x),-1)))
uint32_t program[] = {
    //Main program
    LOAD(R0, 3),
    CALL(7),    //call the "draw the box" subroutine
    SWI(R0),
    HCF,

    //program data
    SOFTWARE_INTERRUPT_SOURCE,
    BOX_WIDTH,
    BOX_HEIGHT,
    STARTING_POSITION_IN_FRAME_BUFFER,
    RED | GREEN | BLUE,

    //Draw the box initialization
    LOAD(PIXEL_POSITION, -3),       //STARTING_POSITION_IN_FRAME_BUFFER
    LOAD(PIXEL_VALUE, -3),          //BLUE
    LOAD(BOX_WIDTH_COUNTER, -7),    //BOX_WIDTH
    LOAD(BOX_HEIGHT_COUNTER, -7),   //BOX_HEIGHT

    //Draw the box
    STORER(PIXEL_VALUE, PIXEL_POSITION, FRAME_BUFFER_START),
    INC(PIXEL_POSITION),
    DEC(BOX_WIDTH_COUNTER),
    BRP(-4),
    ADD_IMMEDIATE(BOX_WIDTH_COUNTER, BOX_WIDTH_COUNTER, BOX_WIDTH),
    ADD_IMMEDIATE(PIXEL_POSITION, PIXEL_POSITION, SCREEN_WIDTH),
    ADD_IMMEDIATE(PIXEL_POSITION, PIXEL_POSITION, -BOX_WIDTH),
    DEC(BOX_HEIGHT_COUNTER),
    BRP(-8),
    
    RETURN
};
#undef SCREEN_WIDTH
#undef STARTING_POSITION_IN_FRAME_BUFFER
#undef BOX_WIDTH
#undef BOX_HEIGHT
#undef FRAME_BUFFER_START
#undef BLUE


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
        for(int i = 0; i< num_steps; i++)
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
    //const int num_steps = 30;
    run(computer, RUN_FOREVER);

    quit_simulation();

    return 0;
}
