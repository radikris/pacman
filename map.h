#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <math.h>
#include <stdlib.h>
#include "maze.h"
#include <stdio.h>
#include "map.h"

typedef struct Map{
    char ** map_array;
    int coin;
    int height;
    int width;
}Map;

Map mapload(char * filename);

void printmap(Map proba);
