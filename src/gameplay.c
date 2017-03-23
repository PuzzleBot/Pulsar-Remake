#include "graphics.h"

extern Wall z_walls[GRIDSIZE][GRIDSIZE-1];
extern Wall x_walls[GRIDSIZE-1][GRIDSIZE];

extern Mob * mobList;

extern void setViewPosition(float, float, float);
extern Boolean playerHasKey;

Meteor * meteorList = NULL;

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
            randomizeAllMobPositions();
            world[*blockX][*blockY][*blockZ] = CUBE_EMPTY;
            break;
        case CUBE_GREEN:
            /*Bounce*/
            world[*blockX][*blockY][*blockZ] = CUBE_EMPTY;
            break;
        case CUBE_LIGHTBLUE:
            /*Meteors*/
            beginStarfall();
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

    generateAllPickups();

    setViewPosition(-(RIGHTWALL-3), -(FLOORHEIGHT+2), -(BOTTOMWALL+3));
}


void generateAllPickups(){
    int i;
    int x, y, z;

    /*Place a key*/
    generateValidSpawnPosition(&x, &y, &z);
    world[x][y - 1][z] = CUBE_WHITE;

    /*Teleport pickups*/
    for(i = 0; i < RED_PICKUPS; i++){
        generateValidSpawnPosition(&x, &y, &z);
        world[x][y - 1][z] = CUBE_RED;
    }

    /*Aerial Faith Plates (Bounce pads)*/
    for(i = 0; i < GREEN_PICKUPS; i++){
        generateValidSpawnPosition(&x, &y, &z);
        world[x][y - 1][z] = CUBE_GREEN;
    }

    /*Meteor pickups*/
    for(i = 0; i < BLUE_PICKUPS; i++){
        generateValidSpawnPosition(&x, &y, &z);
        world[x][y - 1][z] = CUBE_LIGHTBLUE;
    }
}

/*Drop blocks randomly*/
void beginStarfall(){
    int i;

    for(i = 0; i < STARFALL_BLOCKS; i++){
        meteorList = addMeteorToList(meteorList, createNewMeteor());
    }
}

Meteor * createNewMeteor(){
    Meteor * newMeteor = calloc(1, sizeof(Meteor));
    int x, y, z;
    if(newMeteor == NULL){
        printf("Not enough memory!\n");
        exit(1);
    }

    generateValidSpawnPosition(&x, &y, &z);
    newMeteor->x_pos = x;
    newMeteor->y_pos = (rand() % (STARFALL_UPPERBOUND - STARFALL_LOWERBOUND)) + STARFALL_LOWERBOUND;
    newMeteor->z_pos = z;
    newMeteor->y_velocity = -(GRAVITY * 2);
    newMeteor->next = NULL;
    newMeteor->prev = NULL;

    return(newMeteor);
}

Meteor * addMeteorToList(Meteor * list, Meteor * new){
    new->next = list;
    list->prev = new;
    return(new);
}

Meteor * deleteMeteorFromList(Meteor * list, Meteor * toDelete){
    Meteor * newHead = list;

    if(toDelete->prev == NULL){
        newHead = toDelete->next;
    }
    else{
        toDelete->prev->next = toDelete->next;
    }

    if(toDelete->next != NULL){
        toDelete->next->prev = toDelete->prev;
    }

    free(toDelete);

    return(newHead);
}

void destroyMeteorList(Meteor * list){
    Meteor * currentMeteor = list;
    while(currentMeteor != NULL){
        currentMeteor = currentMeteor->next;
        free(list);
        list = currentMeteor;
    }
}
