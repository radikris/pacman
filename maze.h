#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>

SDL_Renderer *renderer;
SDL_Window *window;
SDL_Event ev;

void sdl_init(int width, int height, SDL_Window **pwindow, SDL_Renderer **prenderer);
Uint32 timer(Uint32 ms, void *param);
void test_loop();