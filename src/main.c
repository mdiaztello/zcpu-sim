


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "debug.h"

#include "computer.h"

//This will allow us to write test programs inline without manually encoding
//the instructions. The only caveat is that there is no support for labels, so
//jumps have to be calculated and placed manually.
#include "preprocessor_assembler.h"

// include any SDL stuff here

#include <SDL2/SDL.h>

bool simulation_running = false;

const int init_window_x = 600;
const int init_window_y = 600;
const int window_width = 640;
const int window_height = 480;

//The window we'll be rendering to
SDL_Window *window = NULL;

//The surface contained by the window
SDL_Surface* screen_surface = NULL;


static void program_failure(void)
{
    exit(EXIT_FAILURE);
}

void clean_up(void)
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void quit_simulation(void)
{
    clean_up();
    exit(EXIT_SUCCESS);
}

void init_window(void)
{
    //int init_error = SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
    int init_error = SDL_Init(SDL_INIT_VIDEO);
    if(init_error)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        program_failure();
    }

    window = SDL_CreateWindow("zcpu-sim", init_window_x, init_window_y, window_width, window_height, SDL_WINDOW_SHOWN);
    if(window == NULL)
    {
        program_failure();
    }

    //get window surface
    screen_surface = SDL_GetWindowSurface(window);
}

void input(void)
{
    int speed = 20;
    static size_t ticks = 0;
    SDL_Event e;
    while(SDL_PollEvent(&e))
    {
        switch(e.type)
        {
            case SDL_QUIT:
                simulation_running = false;
                break;
            default:
                break;
        }

    }

    if(e.type == SDL_KEYDOWN)
    {
        if(e.key.keysym.sym == SDLK_q)
        {
            simulation_running = false;
        }
        else
        {
        }
    }


#if 0
    if(ticks > 25)
    {
        ticks = 0;
    }
    else
    {
        ticks++;
    }
#endif
}

void clear_screen(void)
{
    SDL_FillRect(screen_surface, NULL, SDL_MapRGB( screen_surface->format, 0x00, 0x3F, 0x00));
}

void draw(void)
{
    clear_screen();
    SDL_UpdateWindowSurface(window);
}

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

    init_window();

    simulation_running = true;
    while(simulation_running)
    {
        draw();
    }

    quit_simulation();

    return 0;
}
