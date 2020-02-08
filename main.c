#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL_mixer.h>
#include "uj.h"
#include <time.h>
#define BLOCK 25
#include <math.h>

/* kulon fuggvenybe, hogy olvashatobb legyen */
void sdl_init(int width, int height, SDL_Window **pwindow, SDL_Renderer **prenderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("Error while starting SDL: %s", SDL_GetError());
        exit(1);
    }
    SDL_Window *window = SDL_CreateWindow("PAC-MAN",
                                            SDL_WINDOWPOS_CENTERED,
                                            SDL_WINDOWPOS_CENTERED,
                                            width * 25,
                                            height * 25,
                                            0);
    if (window == NULL) {
        SDL_Log("Error while creating window: %s", SDL_GetError());
        exit(1);
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("Error while creating renderer: %s", SDL_GetError());
        exit(1);
    }
    SDL_RenderClear(renderer);

    *pwindow = window;
    *prenderer = renderer;
}
Uint32 timer(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    SDL_PushEvent(&ev);
    return ms;
}

void delay(double masodperc)
{
	/**
	A kesleltetes eleresehez a gep szamolja az idot, csak ezt masodpercbe kell valtani
	*/
	// A secundumot milli_secondumba konvertalni
	double millimasodperc = 1000 * masodperc;

	clock_t start_time = clock();

	// addig noveljuk az idot, amig el nem erjuk a konvertalt millisecet
	while (clock() < start_time + millimasodperc);
}

typedef struct Map{
    char ** map_array;
    int coin;
    int height;
    int width;
}Map;

typedef struct Monsters{
    int x;
    int y;
    SDL_Texture * shape;
    struct Monsters * next;
    char previous;
}Monsters;

Map mapload(char * filename){
    Map start;
    FILE * fp;
    fp=fopen(filename, "r");
    start.width=29;
    start.height=31;


    start.map_array=(char**)malloc(sizeof(char*)*start.height);
    start.map_array[0]=(char*)malloc(sizeof(char)*start.height*start.width);
    for(int i=1; i<start.height; i++){
        start.map_array[i]=start.map_array[0]+i*start.width;
    }

    char c;
        for(int i=0; i<start.height; i++){
            for(int j=0; j<start.width; j++){
                fscanf(fp, "%c", &c);
                start.map_array[i][j]=c;
            }
        }
    return start;
}

void printmap(Map proba){
    for(int i=0; i<proba.height; i++){
        for(int j=0; j<proba.width-1; j++){
            //printf("%c", proba.map_array[i][j]);
            //if(proba.map_array[i][j]=='B')
                //printf("%d es %d --", i, j);
        }
            //printf("\n");
    }
}

typedef enum Direction{UP, DOWN, RIGHT, LEFT} Direction;

void kep(SDL_Renderer *renderer, SDL_Texture *babuk, int x, int y, int BLOCK1, int BLOCK2) {
    /* a forras kepbol ezekrol a koordinatakrol, ilyen meretu reszletet masolunk. */
    SDL_Rect src = { 0, 0, BLOCK1, BLOCK2};
    /* a cel kepre, ezekre a koordinatakra masoljuk */
    SDL_Rect dest = {x, y, BLOCK1, BLOCK2};
    /* kepreszlet masolasa */
    SDL_RenderCopy(renderer, babuk, &src, &dest);
}

void szoveg(SDL_Renderer *renderer, char * szoveg, int xhely, int yhely, int r, int g, int b){
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("LiberationSerif-Regular.ttf", 20);
    if (!font) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(1);
    }
    TTF_Font *kerdesvalasz=TTF_OpenFont("LiberationSerif-Regular.ttf", 13);

    SDL_Color szin = {r, g, b};
    SDL_Surface *felirat;
    SDL_Texture *felirat_t;
    SDL_Rect hova = { 0, 0, 0, 0 };
    felirat = TTF_RenderUTF8_Blended_Wrapped(font, szoveg, szin, 700);   //Solid
    felirat_t = SDL_CreateTextureFromSurface(renderer, felirat);
    if(xhely==900)
        hova.x = (xhely - felirat->w) / 2;
    else
        hova.x=xhely;
    hova.y = yhely;
    hova.w = felirat->w;
    hova.h = felirat->h;

    SDL_RenderCopy(renderer, felirat_t, NULL, &hova);
    SDL_FreeSurface(felirat);
    SDL_DestroyTexture(felirat_t);
}

void printtoscreen(SDL_Renderer * renderer, Map palya, Monsters * redblueorapink, Direction where, SDL_Texture ** players, int bonus, SDL_Texture * mute){
    int whereint=(int) where;
    for(int i=0; i<palya.height; i++){
        for(int j=0; j<palya.width-1; j++){
            if(palya.map_array[i][j]=='W')
                boxRGBA(renderer, j*BLOCK, i*BLOCK, (j+1)*BLOCK, (i+1)*BLOCK, 0, 0, 255, 255);
            else if(palya.map_array[i][j]==' ')
                boxRGBA(renderer, j*BLOCK, i*BLOCK, (j+1)*BLOCK, (i+1)*BLOCK, 0, 0, 0, 255);
            else if(palya.map_array[i][j]=='@')
                kep(renderer, players[whereint], j*BLOCK, i*BLOCK, BLOCK, BLOCK);
            else if(palya.map_array[i][j]=='P')
                filledCircleRGBA(renderer, j*BLOCK+(BLOCK/2), i*BLOCK+(BLOCK/2), 3, 245, 215, 110, 255);
            else if(palya.map_array[i][j]=='$')
                filledCircleRGBA(renderer, j*BLOCK+(BLOCK/2), i*BLOCK+(BLOCK/2), 10, 245, 213, 225, 255);
            else
                boxRGBA(renderer, j*BLOCK, i*BLOCK, (j+1)*BLOCK, (i+1)*BLOCK, 0, 0, 0, 255);
        }
    }
    Monsters * iter=redblueorapink;
    while(iter!=NULL){
        if(bonus>0)
            kep(renderer, players[5], iter->x*BLOCK, iter->y*BLOCK, BLOCK, BLOCK);
        else
            kep(renderer, iter->shape, iter->x*BLOCK, iter->y*BLOCK, BLOCK, BLOCK);
        iter=iter->next;
    }
    kep(renderer, mute, 0, 0, BLOCK, BLOCK);

}

typedef struct{
    int x;
    int y;
}Koordinata;

Koordinata find_shape(Map start, char shape){
    Koordinata player;
    player.x=-1;
    player.y=-1;
    for(int i=0; i<start.height; i++){
        for(int j=0; j<start.width; j++)
            if(start.map_array[i][j]==shape){
                player.x=j;
                player.y=i;
                return player;
            }
    }
    return player;
}

Monsters * create(){
    Monsters * newm=(Monsters*)malloc(sizeof(Monsters));
    newm->next=NULL;
    return newm;
}

bool vanished(Map * start, Koordinata * anything, char shape, int * coin){
    if(anything->x==-1 && anything->y==-1){
        anything->x=13;
        anything->y=13;
        *coin=*coin+500;
        start->map_array[anything->y][anything->x]=shape;
    }
}

Monsters * identifier(Map start, SDL_Texture ** monstershapes, int * coin){
    Monsters * redm=create();
    Monsters * bluem=create();
    Monsters * orangem=create();
    Monsters * pinkm=create();

    Koordinata tempmon;

    tempmon=find_shape(start, '1');
    vanished(&start, &tempmon, '1', coin);
    redm->x=tempmon.x;
    redm->y=tempmon.y;
    redm->previous=' ';
    redm->shape=monstershapes[0];

    tempmon=find_shape(start, '2');
    vanished(&start, &tempmon, '2', coin);
    bluem->x=tempmon.x;
    bluem->y=tempmon.y;
    bluem->previous=' ';
    bluem->shape=monstershapes[1];


    tempmon=find_shape(start, '3');
    vanished(&start, &tempmon, '3', coin);
    orangem->x=tempmon.x;
    orangem->y=tempmon.y;
    orangem->previous=' ';
    orangem->shape=monstershapes[2];

    tempmon=find_shape(start, '4');
    vanished(&start, &tempmon, '4', coin);
    pinkm->x=tempmon.x;
    pinkm->y=tempmon.y;
    pinkm->previous=' ';
    pinkm->shape=monstershapes[3];

    redm->next=bluem;
    bluem->next=orangem;
    orangem->next=pinkm;
    pinkm->next=NULL;

    return redm;
}

bool free_direction(Map start, int direction){
    Koordinata player=find_shape(start, '@');
    if(player.x==-1 && player.y==-1)
        return false;

    if(direction==UP)
        if(start.map_array[player.y-1][player.x]=='W')
            return false;
    if(direction==DOWN)
        if(start.map_array[player.y+1][player.x]=='W')
            return false;
    if(direction==RIGHT)
        if(start.map_array[player.y][player.x+1]=='W')
            return false;
    if(direction==LEFT)
        if(start.map_array[player.y][player.x-1]=='W')
            return false;
    return true;
}

bool ismonster(Map start, int x, int y){
    if(start.map_array[x][y]=='1' || start.map_array[x][y]=='2' || start.map_array[x][y]=='3' || start.map_array[x][y]=='4')
        return true;
    return false;
}

void step(Map * start, int direction, Koordinata * player, int * coin, int * bonus, int * life){
    if(free_direction(*start, direction)==false)
        return;
    //*bonus=0;

    if(direction==UP){
        if(start->map_array[player->y-1][player->x]=='P')
            (*coin)++;
        if(start->map_array[player->y-1][player->x]=='$')
           *bonus=17;

        if(*bonus==0 && ismonster(*start, player->y-1, player->x)){
            start->map_array[player->y][player->x]=' ';
            (*life)--;
        }
        else{
            start->map_array[player->y][player->x]=' ';
            start->map_array[player->y-1][player->x]='@';
            player->y--;
        }
    }else if(direction==DOWN){
        if(start->map_array[player->y+1][player->x]=='P')
            (*coin)++;
        if(start->map_array[player->y+1][player->x]=='$')
           *bonus=17;
        if(*bonus==0 && ismonster(*start, player->y+1, player->x)){
            start->map_array[player->y][player->x]=' ';
            (*life)--;
        }
        else{
        start->map_array[player->y][player->x]=' ';
        start->map_array[player->y+1][player->x]='@';
        player->y++;
        }
    }else if(direction==RIGHT){
            if(start->map_array[player->y][player->x+1]=='P')
                (*coin)++;
            if(start->map_array[player->y][player->x+1]=='$')
                *bonus=17;

            if(*bonus==0 && ismonster(*start, player->y, player->x+1)){
                start->map_array[player->y][player->x]=' ';
                (*life)--;
            }
            else{
                start->map_array[player->y][player->x]=' ';
                start->map_array[player->y][player->x+1]='@';
                player->x++;
            }
            if(player->x>27){
                start->map_array[player->y][player->x]=' ';
                player->x=0;
                start->map_array[player->y][player->x]='@';
            }
    }else if(direction==LEFT){
            if(start->map_array[player->y][player->x-1]=='P')
                (*coin)++;
            if(start->map_array[player->y][player->x-1]=='$')
                *bonus=17;
            if(*bonus==0 && ismonster(*start, player->y, player->x-1)){
                (*life)--;
                start->map_array[player->y][player->x]=' ';
            }
            else{
                start->map_array[player->y][player->x]=' ';
                start->map_array[player->y][player->x-1]='@';
                player->x--;
            }
            if(player->x<0){
                start->map_array[player->y][player->x]=' ';
                player->x=27;
                start->map_array[player->y][player->x]='@';
            }
    }
    if(*bonus>0)
        (*bonus)--;
    return;
}

double distance_calculator(int x, int y){
    int newx, newy;
    newx=(double)x;
    newy=(double)y;
    return sqrt((newx*newx)+(newy*newy));
}

bool controllthespot(Map start, int x, int y){
    if((x<0 || x>27 || y<0 || y>30 || start.map_array[x][y]=='W'))
        return false;
    return true;
}

void random_distance(int *minx, int *miny, int randomnumber, Monsters * redy, Map start){
    switch(randomnumber){
            case 0:
                if(controllthespot(start, *miny-1, *minx)){
                    *miny=redy->y-1;
                    *minx=redy->x;
                }
            break;
            case 1:
                if(controllthespot(start, *miny+1, *minx)){
                    *miny=redy->y+1;
                    *minx=redy->x;
                }
            break;
            case 2:
                if(controllthespot(start, *miny, *minx+1)){
                    *miny=redy->y;
                    *minx=redy->x+1;
                }
            break;
            case 3:
                if(controllthespot(start, *miny, *minx-1)){
                    *miny=redy->y;
                    *minx=redy->x-1;
                }
            break;
        }
}

bool backtobase(int celx, int cely, int prex, int prey, Direction monsterdirection){
    if((prey>=11 && prey<=16) && prex==11 && monsterdirection==DOWN)
        return true;
    return false;
}

void red_movement(Map * start, Koordinata * playerco, Monsters * redblueorapink, char redchar, int * life){
    Monsters * redy=redblueorapink;
    int random;
    Direction monsterdirection=DOWN;
    double minimum_distance=distance_calculator(redy->x-playerco->x, redy->y-playerco->y);
    int miny, minx;
    miny=redy->y;
    minx=redy->x;
    double tempdist;
    if(controllthespot(*start, redy->y-1, redy->x)){   //felfelé +1
        tempdist=distance_calculator(redy->x-playerco->x, redy->y-1-playerco->y);
        if(minimum_distance>tempdist){
            minimum_distance=tempdist;
            miny=redy->y-1;
            minx=redy->x;
            monsterdirection=UP;
        }
    }
    if(controllthespot(*start, redy->y+1, redy->x)){   //lefelé +1
        tempdist=distance_calculator(redy->x-playerco->x, redy->y+1-playerco->y);
        if(minimum_distance>tempdist){
            minimum_distance=tempdist;
            miny=redy->y+1;
            minx=redy->x;
            monsterdirection=DOWN;
        }
    }
    if(controllthespot(*start, redy->y, redy->x+1)){   //jobbra +1
        tempdist=distance_calculator(redy->x+1-playerco->x, redy->y-playerco->y);
        if(minimum_distance>tempdist){
            minimum_distance=tempdist;
            miny=redy->y;
            minx=redy->x+1;
            monsterdirection=RIGHT;
        }
    }
    if(controllthespot(*start, redy->y, redy->x-1)){   //balra +1
        tempdist=distance_calculator(redy->x-1-playerco->x, redy->y-playerco->y);
        if(minimum_distance>tempdist){
            minimum_distance=tempdist;
            miny=redy->y;
            minx=redy->x-1;
            monsterdirection=LEFT;
        }
    }

    if(miny==redy->y && minx==redy->x){
        random=rand()%4;
        random_distance(&minx, &miny, random, redy, *start);
    }

    if(backtobase(miny, minx, redy->y, redy->x, monsterdirection)){
        random=rand()%2;
        switch(random){
            case 0:
                miny=11;
                minx=redy->x+1;
            break;
            case 1:
                miny=11;
                minx=redy->x-1;
            break;
        }
    }
    char temp;
    char tempblock=start->map_array[redy->y][redy->x];
    if(start->map_array[miny][minx]=='P' || start->map_array[miny][minx]=='$'){
        start->map_array[redy->y][redy->x]=' ';
        start->map_array[miny][minx]=redchar;
        //start->map_array[redy->y][redy->x]='';
    }else{
        temp=start->map_array[miny][minx];
        start->map_array[miny][minx]=start->map_array[redy->y][redy->x];
        start->map_array[redy->y][redy->x]=temp;
    }
    if(temp=='@' || tempblock=='@'){
        start->map_array[redy->y][redy->x]=' ';
        printf("Vesztettel!\n");
        (*life)--;
        //exit(0);
    }
    /*
    start->map_array[redy->y][redy->x]=' ';
        start->map_array[miny][minx]=redchar;*/

}

int crisscross(Map start, int x, int y){
    int roads=0;
    if(x-1<0 || x+1>27 || y-1 <0 || y+1>27)
        return 0;
    if(start.map_array[x-1][y]==' ')
        roads++;
    if(start.map_array[x+1][y]==' ')
        roads++;
    if(start.map_array[x][y-1]==' ')
        roads++;
    if(start.map_array[x][y+1]==' ')
        roads++;
    return roads;
}

void pink_movement(Map * start, Koordinata * playerco, Monsters * redblueorapink, char pinkchar, int * life){
    //Monsters * pinky=redblueorapink->next->next->next;
    Monsters * pinky=redblueorapink;
    //double minimum_distance=distance_calculator(pinky->x-playerco->x, pinky->y-playerco->y);
    double minimum_distance=100;
    int miny, minx;
    miny=pinky->y;
    minx=pinky->x;
    double tempdist;
    for(int i=0; i<start->height; i++){
        for(int j=0; j<start->width; j++){
            if(crisscross(*start, i, j)>=3){
                tempdist=distance_calculator(i-playerco->x, j-playerco->y);
                if(minimum_distance>tempdist){
                    minimum_distance=tempdist;
                    miny=j;
                    minx=i;
                }
            }
        }
    }

    Koordinata * crossingco=(Koordinata*)malloc(sizeof(Koordinata));
    crossingco->y=miny;
    crossingco->x=minx;
    red_movement(start, crossingco, pinky, pinkchar, life);
    //printmap(*start);
}

Koordinata triangle_gravitycenter(Koordinata * one, Koordinata * two, Koordinata * three){
    Koordinata center;
    center.x=(one->x+two->x+three->x)/3;
    center.y=(one->y+two->y+three->y)/3;
    //printf("Elso: %d es %d\nMasodik: %d es %d\nHarmadik: %d es %d\n", one->x, one->y, two->x, two->y, three->x, three->y);
    //printf("A center: %d es %d\n", center.x, center.y);
    return center;
}

void orange_movement(Map * start, Koordinata * playerco, Monsters * redblueoranpink, int * life){
    Monsters * redy=redblueoranpink;
    Monsters * pinky=redblueoranpink->next->next->next;
    Koordinata * first=(Koordinata*)malloc(sizeof(Koordinata));
    Koordinata * second=(Koordinata*)malloc(sizeof(Koordinata));
    first->x=redy->x;
    first->y=redy->y;
    second->x=pinky->x;
    second->y=pinky->y;
    Koordinata center=triangle_gravitycenter(first, second, playerco);
    Monsters * orangy=redblueoranpink->next->next;
    red_movement(start, &center, orangy, '3', life);
}

void blue_movement(Map * start, Koordinata * playerco, Monsters * redblueoranpink, int * life){
    Monsters * bluey=redblueoranpink->next;
    int fiftyfifty=rand()%2;
    switch(fiftyfifty){
        case 0:
            red_movement(start, playerco, bluey, '2', life);
            break;
        case 1:
            pink_movement(start, playerco, bluey, '2', life);
            break;
    }
}

bool endofthegame
(Map * start, bool * coinover, int lifes, Koordinata * playerco){
    Koordinata player=find_shape(*start, '@');
    if(player.x==-1 && player.y==-1 && lifes==0){
        *coinover=false;
        return true;
    }else if(player.x==-1 && player.y==-1 && (lifes>0)){
        playerco->x=16;
        playerco->y=26;
        start->map_array[26][16]='@';
        *coinover=false;
        return false;
    }
    int coins=0;
    Koordinata anycoin=find_shape(*start, 'P');
    if(anycoin.x==-1 && anycoin.y==-1){
        *coinover=true;
        return true;
    }
    return false;
}

Direction movement_input(Map start, Direction where);

void automatic_movement(Map *start, Direction where, Koordinata * playerco, Monsters * redblueorapink, SDL_Texture ** players, int * coin, int * bonus, int * life, SDL_Texture * mute);

int main(int argc, char *argv[]) {
    sdl_init(28, 31, &window, &renderer);
    SDL_TimerID id = SDL_AddTimer(150, timer, NULL);

    SDL_Texture *monsters[4];
    monsters[0]=IMG_LoadTexture(renderer, "red_monster.png");
    monsters[1]=IMG_LoadTexture(renderer, "blue_monster.png");
    monsters[2]=IMG_LoadTexture(renderer, "orange_monster.png");
    monsters[3]=IMG_LoadTexture(renderer, "pink_monster.png");

    SDL_Texture *players[6];
    players[0]=IMG_LoadTexture(renderer, "player_up.png");  //UP, down, right, left
    players[1]=IMG_LoadTexture(renderer, "player_down.png");  //UP, down, right, left
    players[2]=IMG_LoadTexture(renderer, "player_right.png");  //UP, down, right, left
    players[3]=IMG_LoadTexture(renderer, "player_left.png");  //UP, down, right, left
    players[4]=IMG_LoadTexture(renderer, "player_closed.jpg");  //UP, down, right, left
    players[5]=IMG_LoadTexture(renderer, "frozenmonster.jpg");  //frozen monster

    SDL_Texture *gameover;
    gameover=IMG_LoadTexture(renderer, "gameover.png");
    SDL_Texture * victory;
    victory=IMG_LoadTexture(renderer, "victory.jpg");

    SDL_Texture *mute;
    mute=IMG_LoadTexture(renderer, "mute.png");

    SDL_INIT_AUDIO;
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Music *backgroundSound = Mix_LoadMUS("pacman_song.wav");
    Mix_Chunk *death = Mix_LoadWAV("pacman_death.wav");
    Mix_Chunk *goal = Mix_LoadWAV("pacman_beginning.wav");

    Map start;
    start=mapload("maze.txt");

    Monsters * redblueoranpink;
    int coin=0;
    int life=3;
    redblueoranpink=identifier(start, monsters, &coin);


    SDL_RenderPresent(renderer);

    printmap(start);
    Koordinata playerco;
    playerco=find_shape(start, '@');

    int bonus=0;
    bool coinover=false;
    Direction where=4;
    Direction previous=-1;
    printtoscreen(renderer, start, redblueoranpink, where, players, bonus, mute);
    /* idozito torlese */
    //SDL_RemoveTimer(id);

    time_t beginning=time(0);
    bool quit=false;
    bool click=false;
    int elozox, elozoy;
    bool musicmute=false;
    int repeat=15;
    while (!quit && life>0) {
        SDL_WaitEvent(&event);
        redblueoranpink=identifier(start, monsters, &coin);

        switch (event.type) {
            case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        click = true;
                        elozox = event.button.x;
                        elozoy = event.button.y;
                    }
                    break;
            case SDL_MOUSEBUTTONUP:
                    if (click)
                        if((elozox>=0 && elozox<=BLOCK) && (elozoy>=0 && elozoy<=BLOCK)){
                            if(musicmute){
                                musicmute=false;
                                repeat=15;
                            }else{
                                musicmute=true;
                                repeat=0;
                            }
                        }

            case SDL_KEYDOWN:
                where=movement_input(start, where);
                if(previous!=where && !musicmute){
                    Mix_PlayMusic(backgroundSound, repeat);
                }
                previous=where;
                break;

            case SDL_USEREVENT:
                    if(bonus>0){
                        automatic_movement(&start, where, &playerco, redblueoranpink, players, &coin, &bonus, &life, mute);
                    }else{
                        automatic_movement(&start, where, &playerco, redblueoranpink, players, &coin, &bonus, &life, mute);
                        red_movement(&start, &playerco, redblueoranpink, '1', &life);
                        pink_movement(&start, &playerco, redblueoranpink->next->next->next, '4', &life);
                        orange_movement(&start, &playerco, redblueoranpink, &life);
                        blue_movement(&start, &playerco, redblueoranpink, &life);
                    }
                    SDL_RenderPresent(renderer);
                break;

            case SDL_QUIT:
                quit = true;
                break;
        }
        char cointoprint[50];
        time_t end=time(0)-beginning;
        sprintf(cointoprint, "Coins: %d     Lifes: %d     Time: %d", coin, life, end);
        szoveg(renderer, cointoprint, 250, 750, 255, 255, 255);
         if(endofthegame(&start, &coinover, life, &playerco))
            quit=true;
        SDL_RenderPresent(renderer);
    }

    if(coinover){
        kep(renderer, victory, 200, 325, 300, 100);
        Mix_PlayChannel(-1, goal, 0);
    }
    else{
            kep(renderer, gameover, 200, 325, 300, 100);
            Mix_PlayChannel(-1, death, 0);
    }

    SDL_RenderPresent(renderer);
    delay(3);
    SDL_Quit();

    return 0;
}

Direction movement_input(Map start, Direction where){
        switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        if(free_direction(start, LEFT))
                            where = LEFT;
                    break;
                    case SDLK_RIGHT:
                        if(free_direction(start, RIGHT))
                            where = RIGHT;
                         break;
                    case SDLK_UP:
                        if(free_direction(start, UP))
                            where = UP;
                        break;
                    case SDLK_DOWN:
                        if(free_direction(start, DOWN))
                            where = DOWN;
                        break;
                }
        return where;
}

void automatic_movement(Map *start, Direction where, Koordinata * playerco, Monsters * redblueorapink, SDL_Texture ** players, int * coin, int * bonus, int * life, SDL_Texture * mute){
    step(start, where, playerco, coin, bonus, life);
    printtoscreen(renderer, *start, redblueorapink, where, players, *bonus, mute);
    SDL_RenderPresent(renderer);
}
