#include "graphics.h"
#include <stdio.h>
#include <sys/time.h>

Wall z_walls[GRIDSIZE][GRIDSIZE+1];
Wall x_walls[GRIDSIZE+1][GRIDSIZE];
AnimationList * animationQueue;

time_t previousUpdate;

extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);

int main(){
    int i;
    int j;
    int k;
    
    int rightwall;
    int topwall;
    
    time_t currentTime;
    double timeSinceUpdate;
    
    int wallTimer = 0;
    int wallAnimationTimer = 0;
    
    for(i = 0; i < WORLDX; i++){
        for(j = 0; j < WORLDY; j++){
            for(k = 0; k < WORLDZ; k++){
                world[i][j][k] = 0;
            }
        }
    }
    
    rightwall = LEFTWALL + (ROOMSIZE * GRIDSIZE) + GRIDSIZE;
    topwall = BOTTOMWALL + (ROOMSIZE * GRIDSIZE) + GRIDSIZE;
    
    world[LEFTWALL][FLOORHEIGHT+3][BOTTOMWALL] = 3;
    
    /*Build the floor*/
    for(i = LEFTWALL; i <= rightwall; i++){
        for(j = BOTTOMWALL; j <= topwall; j++){
            world[i][FLOORHEIGHT][j] = 4;
        }
    }
    
    /*Build the pillars*/
    for(i = LEFTWALL; i <= rightwall; i = i + ROOMSIZE + 1){
        for(j = BOTTOMWALL; j <= topwall; j = j + ROOMSIZE + 1){
            world[i][FLOORHEIGHT+1][j] = 2;
            world[i][FLOORHEIGHT+2][j] = 2;
        }
    }
    
    /*Bottom and top borders*/
    for(i = LEFTWALL; i <= rightwall; i++){
        world[i][FLOORHEIGHT+1][BOTTOMWALL] = 2;
        world[i][FLOORHEIGHT+2][BOTTOMWALL] = 2;
        world[i][FLOORHEIGHT+1][topwall] = 2;
        world[i][FLOORHEIGHT+2][topwall] = 2;
    }
    
    /*Left and right borders*/
    for(j = BOTTOMWALL; j <= topwall; j++){
        world[LEFTWALL][FLOORHEIGHT+1][j] = 2;
        world[LEFTWALL][FLOORHEIGHT+2][j] = 2;
        world[rightwall][FLOORHEIGHT+1][j] = 2;
        world[rightwall][FLOORHEIGHT+2][j] = 2;
    }
    
    previousUpdate = time(&previousUpdate);
    return(0);
    
    for(i=0; i < 10000; i++){
        currentTime = time(&currentTime);
        timeSinceUpdate = difftime(currentTime, previousUpdate);
        if(timeSinceUpdate >= 0.0333){
            /*Gravity*/
            float viewX;
            float viewY;
            float viewZ;
            
            wallTimer++;
            if(wallTimer >= 5){
                wallTimer = 0;
                toggleRandomWall();
            }
            
            wallAnimationTimer++;
            if(wallAnimationTimer >= 3){
                wallAnimationTimer = 0;
                processAllAnimations();
                printf("Process %d\n", i);
            }
            
            previousUpdate = time(&previousUpdate);
        }
    }
}
