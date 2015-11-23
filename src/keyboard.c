
// ----------------------------------------------------------------------------
//
//  FILE: keyboard.c
//
//  DESCRIPTION: This module simulates a virtual keyboard interface for the
//  computer simulator. It can request interrupts from the CPU when a key is
//  pressed. The CPU will then jump to the keyboard's interrupt service routine
//  to handle reception of the new character that is transmitted by the key
//  press. The CPU will interact with the keyboard "hardware" via special
//  memory-mapped registers whose addresses are defined in
//  include/memory_map.h.
//
//  On the user interface side of this simulator, the virtual keyboard hardware
//  is connected to real keyboard hardware via SDL. SDL can capture keyboard
//  events from the OS which we can then pass along to the simulator via
//  interrupt requests and memory-mapped registers.
//
// ----------------------------------------------------------------------------



#include "SDL.h"
#include "memory_bus.h"
#include "keyboard.h"


//FIXME: THIS IS A FILTHY HACK! KILL IT WITH FIRE!
//seriously though, I just want to get a way up and running to gracefully
//terminate the program. I'll figure out the proper way later
extern bool simulation_running;


//FIXME: I'm not sure what else to add here just yet, we'll figure that out
//later
struct keyboard_t
{
    //FIXME: do we want to have more than just this one register for
    //communicating with the keyboard? Do we even need a keyboard status
    //register if the keyboard can interrupt the CPU to tell it there is new
    //data? The memory map currently has space allocated for 2 keyboard
    //registers, but I think I may end up only using one
    uint16_t keycode;
};

keyboard_t* create_keyboard(void)
{
    keyboard_t* keyboard = calloc(1, sizeof(struct keyboard_t));
    return keyboard;
}

void destroy_keyboard(keyboard_t* keyboard)
{
    free(keyboard);
}


//This is all debug code for now, we will have to revise this later
void input(keyboard_t* keyboard)
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
            keyboard->keycode = e.key.keysym.sym;
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


void keyboard_cycle(keyboard_t* keyboard, memory_bus_t* bus)
{
    if(KEYBOARD_SELECTED != bus_get_selected_device(bus) || !bus_is_enabled(bus))
    {
        return;
    }

    //it makes no sense to write data _to_ the keyboard, so only reading is supported
    if(bus_is_read_operation(bus))
    {
        bus_set_data_lines(bus, keyboard->keycode);
    }
    //FIXME: is this even needed if we are only writing to a block of memory/a device?
    bus_set_device_ready(bus); //read/write complete
}
