
#include "computer.h"
#include "cpu.h"
#include "memory_bus.h"
#include "memory.h"
#include "graphics.h"
#include "keyboard.h"
#include "timer.h"
#include "interrupt_controller.h"
#include "debug.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>


bool simulation_running = false;

struct computer_t 
{
    uint64_t elapsed_cycles;
    cpu_t* cpu;
    memory_bus_t* bus;
    memory_t* RAM;
    graphics_t* screen;
    keyboard_t* keyboard;
    timer_t* system_timer;
    interrupt_controller_t* interrupt_controller;
};

//FIXME: these will need parameters for graphics and memory_bus later
//Creates a computer object and connects its dependencies, but doesn't 
//initialize it
computer_t* make_computer(cpu_t* cpu, 
                          memory_t* memory, 
                          memory_bus_t* bus, 
                          graphics_t* graphics, 
                          keyboard_t* keyboard, 
                          timer_t* system_timer, 
                          interrupt_controller_t* ic)
{
    computer_t* computer = calloc(1, sizeof(struct computer_t));
    computer->cpu = cpu;
    computer->RAM = memory;
    computer->bus = bus;
    computer->screen = graphics;
    computer->keyboard = keyboard;
    computer->system_timer = system_timer;
    computer->interrupt_controller = ic;
    return computer;
}

//This is our CPU "factory" function, which handles the initialization and dependency
//injection to the CPU "constructor" separately
cpu_t* build_cpu(memory_bus_t* bus, interrupt_controller_t* ic)
{
    cpu_t* cpu = make_cpu(bus, ic);
    init_cpu(cpu);

    return cpu;
}

//creates a new computer system complete with all subsystems
//and initializes/resets it
computer_t* build_computer(void)
{
    const uint16_t DISPLAY_WIDTH = 640;
    const uint16_t DISPLAY_HEIGHT = 480;
    const uint32_t NUM_MEM_LOCATIONS = 1024;
    memory_bus_t* bus = make_memory_bus();
    interrupt_controller_t* ic = make_interrupt_controller();

    cpu_t* cpu = build_cpu(bus, ic);
    memory_t* RAM = make_memory(NUM_MEM_LOCATIONS);
    //DEBUG
    graphics_t* display = create_graphics_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    keyboard_t* keyboard = create_keyboard();
    timer_t* sys_timer = make_timer(IRQ_1);

    computer_t* computer = make_computer(cpu, RAM, bus, display, keyboard, sys_timer, ic);
    computer_reset(computer);
    return computer;
}

void computer_reset(computer_t* computer)
{
    computer->elapsed_cycles = 0;
    cpu_reset(computer->cpu);
    memory_reset(computer->RAM);
    graphics_reset(computer->screen);
    //reset_IO(computer->IO);
    //reset_memory_bus(computer->memory_bus);
}

//load the supplied program into computer memory
void computer_load_program(computer_t* computer, uint32_t* program, size_t program_length)
{
    for(size_t i = 0; i < program_length; i++)
    {
        memory_set(computer->RAM, i, program[i]);
    }
}

uint32_t cycles = 0;
//execute the next single instruction for the program in memory
void computer_single_step(computer_t* computer)
{
    do
    {
        cpu_cycle(computer->cpu);

        bus_cycle(computer->bus);

        memory_cycle(computer->RAM, computer->bus);
        graphics_cycle(computer->screen, computer->bus);
        timer_cycle(computer->system_timer, computer->bus, computer->interrupt_controller);

        computer->elapsed_cycles++;
        cycles++;
    }
    while(!cpu_completed_instruction(computer->cpu));
}

//execute the program in memory until told to stop
void computer_run(computer_t* computer)
{
    const uint32_t MAX_FPS = 60;
    const uint32_t MAX_TIME_BETWEEN_FRAMES_MILLISECONDS = (1000 / MAX_FPS);
    uint32_t frame_time = 0;
    uint32_t old_frame_time = 0;

    uint32_t timestamp = SDL_GetTicks();
    simulation_running = true;
    while(simulation_running)
    {
        computer_single_step(computer);
        
        //We are limiting updating the display and taking keyboard input to a
        //60Hz rate because before we were executing these functions at every
        //opportunity, and they cause the rest of the simulation to slow down.
        //They keyboard input is also done at 60Hz because it seems like a
        //reasonable rate to gather events since nobody can possibly type that
        //fast.
        frame_time = SDL_GetTicks();
        if(MAX_TIME_BETWEEN_FRAMES_MILLISECONDS < (frame_time - old_frame_time))
        {
            old_frame_time = frame_time;
            graphics_draw(computer->screen);
            input(computer->keyboard);
        }

        if(SDL_GetTicks() - timestamp >= 1000)
        {
            printf("%d cycles processed \n", cycles);
            cycles = 0;
            timestamp = SDL_GetTicks();
        }
    }
}

void dump_computer_cpu_state(computer_t* computer)
{
    dump_cpu_state(computer->cpu);
}

void dump_computer_memory(computer_t* computer, size_t starting_address, size_t ending_address)
{
    memory_print(computer->RAM, starting_address, ending_address);
}

void computer_print_elapsed_cycles(computer_t* computer)
{
    printf("the number of elapsed cycles is now %" PRIu64 "\n", computer->elapsed_cycles);
}


