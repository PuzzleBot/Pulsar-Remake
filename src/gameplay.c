#include "graphics.h"

extern Wall z_walls[GRIDSIZE][GRIDSIZE-1];
extern Wall x_walls[GRIDSIZE-1][GRIDSIZE];

extern void setViewPosition(float, float, float);
extern Boolean playerHasKey;

/*For handling pickups and keys on the ground*/
void handleSingleBlock(int * blockX, int * blockY, int * blockZ){
    switch(world[*blockX][*blockY][*blockZ]){
        case CUBE_WHITE:
            /*Key*/
            playerHasKey = TRUE;
            printf("Key get!\n");
            world[*blockX][*blockY][*blockZ] = CUBE_EMPTY;
            break;
        case CUBE_RED:
            /*Random Teleport*/
            world[*blockX][*blockY][*blockZ] = CUBE_EMPTY;
            break;
        case CUBE_GREEN:
            /*Bounce*/
            world[*blockX][*blockY][*blockZ] = CUBE_EMPTY;
            break;
        case CUBE_BLUE:
            /*Meteors*/
            world[*blockX][*blockY][*blockZ] = CUBE_EMPTY;
            break;
        default:
            /*Nothing special, just step on top of the block*/
            *blockY = *blockY + 1;
            break;
    }
}

void mazeDoor(){
    int i, j, k;
    if(playerHasKey == TRUE){
        printf("Maze complete!\n");
        playerHasKey = FALSE;

        for(i = LEFTWALL; i < RIGHTWALL; i++){
            for(j = FLOORHEIGHT; j < FLOORHEIGHT + WALL_HEIGHT + 1; j++){
                for(k = BOTTOMWALL; k < TOPWALL; k++){
                    world[i][j][k] = CUBE_EMPTY;
                }
            }
        }

        resetMaze();
    }
}

void resetMaze(){
    int i, j;

    buildStaticObjects();
    /*Initialize the walls array*/
    for(i = 0; i < GRIDSIZE+1; i++){
        for(j = 0; j < GRIDSIZE; j++){
            x_walls[i][j].state = OPEN;
            z_walls[j][i].state = OPEN;
        }
    }
    initializeWalls();
    initWaypointGrid();
    initializeBulletArray();
    worldMobInit();

    setViewPosition(-(RIGHTWALL-3), -(FLOORHEIGHT+2), -(BOTTOMWALL+3));
}
