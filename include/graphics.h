

#ifndef __GRAPHICS_H_
#define __GRAPHICS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct graphics_t graphics_t;

graphics_t* create_graphics_display(uint16_t width, uint16_t height);
void graphics_destroy(graphics_t* graphics);
//updates the contents of a pixel within the framebuffer; this is our memory
//bus's interface to the graphics subsystem
void graphics_update(graphics_t* graphics, uint32_t pixel_address, uint32_t RGBA_pixel);
//renders the frame buffer contents to the screen
void graphics_draw(graphics_t* graphics);
void graphics_reset(graphics_t* graphics);

#endif //__GRAPHICS_H_
