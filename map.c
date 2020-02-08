#include "map.h"

Map mapload(char * filename){
    Map start;
    FILE * fp;
    fp=fopen(filename, "r");
    start.width=31;
    start.height=29;


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
        for(int j=0; j<proba.width; j++)
            printf("%c", proba.map_array[i][j]);
    }
    printf("\n");
}
