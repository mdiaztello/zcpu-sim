
#ifndef __KEYBOARD_H_
#define __KEYBOARD_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct keyboard_t keyboard_t;

keyboard_t* create_keyboard(void);
void destroy_keyboard(keyboard_t* keyboard);

void input(keyboard_t* keyboard);
void keyboard_cycle(keyboard_t* keyboard, memory_bus_t* bus);

#endif //__KEYBOARD_H_
