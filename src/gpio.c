
// ----------------------------------------------------------------------------
//
//  FILE: gpio.c
//
//  DESCRIPTION: This module implements a general-purpose digital I/O port for
//  the computer so that we can send out digital control signals from the
//  processor, or read digital signals from the outside world. This kind of
//  hardware is typically found on microcontrollers, so I thought I'd include
//  it in the simulator to reflect the possibility that I might do an
//  embedded-systems type project with my custom CPU.
//
//  Applications for a GPIO port on a microcontroller include bit-banged serial
//  communication, LED controls, LCD control, bit-banged PWM, etc.
//
// ----------------------------------------------------------------------------

#include <stdint.h>
#include <stddef.h>  //for size_t
#include "bit_twiddling.h"



struct gpio_port_t
{
    uint32_t direction_register;
    uint32_t data_register;
};

typedef struct gpio_port_t gpio_port_t;

enum port_pin_direction_t
{
    PORT_PIN_INPUT_MODE,
    PORT_PIN_OUTPUT_MODE
};

typedef enum port_pin_direction_t port_pin_direction_t;


static port_pin_direction_t get_pin_direction(gpio_port_t* port, size_t pin_num);



void gpio_port_cycle(gpio_port_t* port)
{
    const int PORT_WIDTH = 32;
    for(int i = 0; i < PORT_WIDTH; i++)
    {
        if(get_pin_direction(port, i) == PORT_PIN_INPUT_MODE)
        {
            //sample the data from the port pin
        }
        else
        {
            //output the data to the port pin
        }
    }
}

static port_pin_direction_t get_pin_direction(gpio_port_t* port, size_t pin_num)
{
    port_pin_direction_t direction = PORT_PIN_INPUT_MODE;
    if(CHECK_BIT_SET(port->direction_register, pin_num))
    {
        direction = PORT_PIN_OUTPUT_MODE;
    }

    return direction;
}
