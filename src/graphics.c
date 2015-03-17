

//This module will handle the display subsystem for the computer. It will
//handle the double-buffering and act transparently for the CPU so that the CPU
//can just write to the frame buffer in its memory map and everything will just
//work.
//
//All of the SDL code necessary to make this work will be encapsulated here

//The bulk of this code was gleaned from http://lazyfoo.net/tutorials/SDL/ and
//https://wiki.libsdl.org/MigrationGuide because prior to today (2015-03-16), I
//had no idea how to use SDL!

// include any SDL stuff here
#include <SDL2/SDL.h>
#include "graphics.h"

struct graphics_t 
{
    uint16_t WINDOW_WIDTH;
    uint16_t WINDOW_HEIGHT;
    //This is where our custom computer will write the graphical output
    uint32_t* frame_buffer;

    //The window we'll be rendering to
    SDL_Window* window;
    //the renderer is an SDL2 concept that handles getting our data to the GPU
    SDL_Renderer* renderer;
    //This texture is where we will copy our framebuffer to for SDL to do its magic
    SDL_Texture* screen;
};


static void init_window(graphics_t* graphics);
static void program_failure(void);

static void change_color(graphics_t* graphics, uint32_t pixel_value);
static void clear_screen(graphics_t* graphics);

//creates our display for the program and initializes the frame buffer and the
//SDL subsystem
graphics_t* create_graphics_display(uint16_t width, uint16_t height)
{
    graphics_t* graphics = calloc(1, sizeof(graphics_t));
    graphics->WINDOW_HEIGHT = height;
    graphics->WINDOW_WIDTH = width;

    //FIXME: eventually need to handle the double buffering
    uint32_t buffer_size = graphics->WINDOW_WIDTH * graphics->WINDOW_HEIGHT * sizeof(uint32_t);
    graphics->frame_buffer = calloc(1, buffer_size);
    if(graphics->frame_buffer == NULL)
    {
        fprintf(stderr, "failed to allocate frame buffer\n");
        program_failure();
    }
    init_window(graphics);

    return graphics;
}

//de-allocates all of the display resources
void graphics_destroy(graphics_t* graphics)
{
    SDL_DestroyWindow(graphics->window);
    SDL_DestroyTexture(graphics->screen);
    SDL_DestroyRenderer(graphics->renderer);
    SDL_Quit();
    free(graphics->frame_buffer);
    free(graphics);
}

void graphics_update(graphics_t* graphics, uint32_t pixel_address, uint32_t RGBA_pixel)
{
    //FIXME: eventually need double-buffering
    int pitch = graphics->WINDOW_WIDTH*sizeof(uint32_t);
    SDL_LockTexture(graphics->screen, NULL, (void**) &(graphics->frame_buffer), &pitch);
    //graphics->frame_buffer[pixel_address] = RGBA_pixel;
    change_color(graphics, 0x00FF00FF);
    SDL_UnlockTexture(graphics->screen);
}


void graphics_reset(graphics_t* graphics)
{
    clear_screen(graphics);
    size_t fb_size = graphics->WINDOW_WIDTH * graphics->WINDOW_HEIGHT * sizeof(uint32_t);
    memset(graphics->frame_buffer, 0x00, fb_size);
}

static void program_failure(void)
{
    exit(EXIT_FAILURE);
}

static void init_window(graphics_t* graphics)
{
    printf("init_window started\n");
    int init_error = SDL_Init(SDL_INIT_VIDEO);
    if(init_error)
    {
        fprintf(stderr, "%s\n", SDL_GetError());
        program_failure();
    }

    graphics->window = SDL_CreateWindow("zcpu-sim", 
                                        SDL_WINDOWPOS_CENTERED, 
                                        SDL_WINDOWPOS_CENTERED, 
                                        graphics->WINDOW_WIDTH, 
                                        graphics->WINDOW_HEIGHT, 
                                        SDL_WINDOW_SHOWN);
    if(graphics->window == NULL)
    {
        fprintf(stderr, "failed to allocate SDL window\n");
        program_failure();
    }

    graphics->renderer = SDL_CreateRenderer(graphics->window, -1, 0);
    if(graphics->renderer == NULL)
    {
        fprintf(stderr, "failed to allocate SDL renderer\n");
        program_failure();
    }

    graphics->screen = SDL_CreateTexture(graphics->renderer, 
                                        SDL_PIXELFORMAT_RGBA8888, 
                                        SDL_TEXTUREACCESS_STREAMING, 
                                        graphics->WINDOW_WIDTH, 
                                        graphics->WINDOW_HEIGHT);
    if(graphics->screen == NULL)
    {
        fprintf(stderr, "failed to allocate SDL texture\n");
        program_failure();
    }

    printf("init_window finished\n");
}

//this is just a little function to help me debug this code; it just fills the
//framebuffer with the requested RGBA encoded pixel value
static void change_color(graphics_t* graphics, uint32_t pixel_value)
{
    for(int col = 0; col < graphics->WINDOW_WIDTH; col++)
    {
        for(int row = 0; row < graphics->WINDOW_HEIGHT; row++)
        {
            graphics->frame_buffer[row*graphics->WINDOW_WIDTH + col] = pixel_value;
        }
    }
}

static void clear_screen(graphics_t* graphics)
{
    SDL_SetRenderDrawColor(graphics->renderer, 0x00, 0x00, 0x00, 0xFF); //clear to black
    SDL_RenderClear(graphics->renderer);
}

void graphics_draw(graphics_t* graphics)
{
    clear_screen(graphics);

    //"blit" the framebuffer to the screen using SDL 2.0 GPU magic
    graphics_update(graphics, 0, 0);
    int pitch = graphics->WINDOW_WIDTH*sizeof(uint32_t);
    SDL_UpdateTexture(graphics->screen, NULL, graphics->frame_buffer, pitch);
    SDL_RenderCopy(graphics->renderer, graphics->screen, NULL, NULL);
    SDL_RenderPresent(graphics->renderer);
}
