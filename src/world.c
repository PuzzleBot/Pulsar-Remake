#include "graphics.h"

HighGrid waypointGrid[(GRIDSIZE * 2)][(GRIDSIZE * 2)];

extern Wall z_walls[GRIDSIZE][GRIDSIZE-1];
extern Wall x_walls[GRIDSIZE-1][GRIDSIZE];

void getCorrespondingHighGridIndex(int * xIndex, int * yIndex, int xBlock, int zBlock){
    if(((xBlock - LEFTWALL + 1) % (ROOMSIZE+1) <= 2) || ((zBlock - BOTTOMWALL + 1) % (ROOMSIZE+1) <= 2)){
        /*Wall*/
        *xIndex = (((xBlock - LEFTWALL + 1) / (ROOMSIZE+1)) * 2) - 1;
        *yIndex = (((zBlock - BOTTOMWALL + 1) / (ROOMSIZE+1)) * 2) - 1;
        printf("Index - wall mode -");
    }
    else{
        /*Empty cell*/
        *xIndex = ((xBlock - LEFTWALL) / (ROOMSIZE+1)) * 2;
        *yIndex = ((zBlock - BOTTOMWALL) / (ROOMSIZE+1)) * 2;
        printf("Index - cell mode -");
    }

    if(*xIndex < 0) *xIndex = 0;
    if(*yIndex < 0) *yIndex = 0;
}

void initWaypointGrid(){
    int i, j;

    for(i = 0; i < (GRIDSIZE * 2) - 1; i++){
        for(j = 0; j < (GRIDSIZE * 2) - 1; j++){
            if((i % 2 == 1) && (j % 2 == 1)){
                /*Pillars connected to walls*/
                waypointGrid[i][j].type = HIGHGRID_PILLAR;
                waypointGrid[i][j].correspondingWall = NULL;
            }
            else if(i % 2 == 1){
                waypointGrid[i][j].type = HIGHGRID_WALL;
                waypointGrid[i][j].correspondingWall = &x_walls[(i/2)][(j/2)];
            }
            else if(j % 2 == 1){
                waypointGrid[i][j].type = HIGHGRID_WALL;
                waypointGrid[i][j].correspondingWall = &z_walls[(i/2)][(j/2)];
            }
            else{
                /*Empty spaces separated by walls*/
                waypointGrid[i][j].type = HIGHGRID_CELL;
                waypointGrid[i][j].correspondingWall = NULL;
            }
        }
    }

    //printWaypointGrid();
}

void printWaypointGrid(){
    int i, j;

    for(i = (GRIDSIZE * 2) - 2; i >= 0; i--){
        for(j = (GRIDSIZE * 2) - 2; j >= 0; j--){
            switch(waypointGrid[i][j].type){
                case HIGHGRID_PILLAR:
                    printf("X");
                    break;
                case HIGHGRID_CELL:
                    printf(" ");
                    break;
                case HIGHGRID_WALL:
                    if(waypointGrid[i][j].correspondingWall->state == CLOSED) {
                        if(i % 2 == 1){
                            printf("-");
                        }
                        else if(j % 2 == 1){
                            printf("|");
                        }
                    }
                    else if(waypointGrid[i][j].correspondingWall->state == OPEN){
                        printf(" ");
                    }
                    else{
                        printf("?");
                    }
                    break;
                default:
                    printf(" ");
                    break;
            }
        }
        printf("G\n");
    }

    printf("\n");
}
