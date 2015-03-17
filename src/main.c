


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

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

//This is where our custom computer will write the graphical output
uint32_t* frame_buffer = NULL;

//The window we'll be rendering to
SDL_Window* window = NULL;

//the renderer is an SDL2 concept that handles getting our data to the GPU
SDL_Renderer* renderer = NULL;
//This texture is where we will copy our framebuffer to for SDL to do its magic
SDL_Texture* screen = NULL;


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
    int init_error = SDL_Init(SDL_INIT_VIDEO);
    if(init_error)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        program_failure();
    }

    window = SDL_CreateWindow("zcpu-sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if(window == NULL)
    {
        fprintf(stderr, "failed to allocate SDL window\n");
        program_failure();
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if(renderer == NULL)
    {
        fprintf(stderr, "failed to allocate SDL renderer\n");
        program_failure();
    }

    screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);
    if(screen == NULL)
    {
        fprintf(stderr, "failed to allocate SDL texture\n");
        program_failure();
    }

    frame_buffer = calloc(1, WINDOW_WIDTH*WINDOW_HEIGHT*sizeof(uint32_t));
    if(frame_buffer == NULL)
    {
        fprintf(stderr, "failed to allocate frame buffer\n");
        program_failure();
    }
}

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

void clear_screen(void)
{
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF); //clear to black
    SDL_RenderClear(renderer);
}

void change_color(uint32_t* buffer, uint32_t pixel_value)
{
    for(int col = 0; col < WINDOW_WIDTH; col++)
    {
        for(int row = 0; row < WINDOW_HEIGHT; row++)
        {
            buffer[row*WINDOW_WIDTH + col] = pixel_value;
        }
    }
}

void draw(void)
{
    clear_screen();

    //FIXME: figure out how to do this locking/unlocking cleanly (i.e. without
    //having my cpu code getting contaminated with a bunch of SDL code)
    int pitch = WINDOW_WIDTH*sizeof(uint32_t);
    SDL_LockTexture(screen, NULL, (void**) &frame_buffer, &pitch);
    change_color(frame_buffer, 0x00FF00FF);
    SDL_UnlockTexture(screen);
    SDL_UpdateTexture(screen, NULL, frame_buffer, WINDOW_WIDTH*sizeof(uint32_t));
    SDL_RenderCopy(renderer, screen, NULL, NULL);
    SDL_RenderPresent(renderer);
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
        input();
        draw();
    }

    quit_simulation();

    return 0;
}
