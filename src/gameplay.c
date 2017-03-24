#include "graphics.h"

extern Wall z_walls[GRIDSIZE][GRIDSIZE-1];
extern Wall x_walls[GRIDSIZE-1][GRIDSIZE];

extern Mob * mobList;

extern void setViewPosition(float, float, float);
extern Boolean playerHasKey;

Meteor * meteorList = NULL;

/*For handling pickups and keys on the ground*/
void handleSingleBlock(int * blockX, float * newY, int * blockZ){
    switch(world[*blockX][(int)*newY][*blockZ]){
        case CUBE_WHITE:
            /*Key*/
            playerHasKey = TRUE;
            printf("Key get!\n");
            world[*blockX][(int)*newY][*blockZ] = CUBE_EMPTY;
            break;
        case CUBE_RED:
            /*Random Teleport*/
            randomizeAllMobPositions();
            world[*blockX][(int)*newY][*blockZ] = CUBE_EMPTY;
            //*newY = *newY + 1;
            break;
        case CUBE_GREEN:
            /*Bounce*/
            world[*blockX][(int)*newY][*blockZ] = CUBE_EMPTY;
            //*newY = *newY + 1;
            break;
        case CUBE_LIGHTBLUE:
            /*Meteors*/
            beginStarfall();
            world[*blockX][(int)*newY][*blockZ] = CUBE_EMPTY;
            //*newY = *newY + 1;
            break;
        default:
            /*Nothing special, just step on top of the block*/
            *newY = *newY + 1;
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
    destroyMeteorList(meteorList);
    meteorList = NULL;

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

/*Meteor functions start here
  Drop blocks randomly*/
void beginStarfall(){
    int i;

    for(i = 0; i < STARFALL_BLOCKS; i++){
        meteorList = addMeteorToList(meteorList, createNewMeteor());
    }
}

Meteor * createNewMeteor(){
    Meteor * newMeteor = calloc(1, sizeof(Meteor));
    int y;
    if(newMeteor == NULL){
        printf("Not enough memory!\n");
        exit(1);
    }

    generateValidSpawnPosition(&newMeteor->x_block, &y, &newMeteor->z_block);
    newMeteor->y_pos = (rand() % (STARFALL_UPPERBOUND - STARFALL_LOWERBOUND)) + STARFALL_LOWERBOUND;
    newMeteor->y_block = (int)newMeteor->y_pos;
    newMeteor->y_velocity = -(GRAVITY * 2);
    newMeteor->next = NULL;
    newMeteor->prev = NULL;

    return(newMeteor);
}

Meteor * addMeteorToList(Meteor * list, Meteor * new){
    new->next = list;

    if(list != NULL){
        list->prev = new;
    }
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
        world[currentMeteor->x_block][currentMeteor->y_block][currentMeteor->z_block] = CUBE_EMPTY;
        currentMeteor = currentMeteor->next;
        free(list);
        list = currentMeteor;
    }
}


Meteor * animateAllMeteors(Meteor * meteorList){
    Meteor * currentMeteor = meteorList;

    while(currentMeteor != NULL){
        world[currentMeteor->x_block][currentMeteor->y_block][currentMeteor->z_block] = CUBE_EMPTY;
        currentMeteor->y_pos = currentMeteor->y_pos + currentMeteor->y_velocity;
        currentMeteor->y_block = (int)currentMeteor->y_pos;
        if(world[currentMeteor->x_block][currentMeteor->y_block][currentMeteor->z_block] != CUBE_EMPTY){
            if(world[currentMeteor->x_block][currentMeteor->y_block][currentMeteor->z_block] == CUBE_BLACK
               || world[currentMeteor->x_block][currentMeteor->y_block][currentMeteor->z_block] == CUBE_METEOR){
                /*The meteor has reached the ground*/
                currentMeteor->y_pos = currentMeteor->y_pos - (currentMeteor->y_velocity * 1.001);
                currentMeteor->y_block = (int)currentMeteor->y_pos;
                world[currentMeteor->x_block][currentMeteor->y_block][currentMeteor->z_block] = CUBE_METEOR;
                meteorList = deleteMeteorFromList(meteorList, currentMeteor);
            }
            else{
                /*Something is in the way, but won't be for long, don't move*/
                currentMeteor->y_pos = currentMeteor->y_pos - (currentMeteor->y_velocity * 1.001);
                currentMeteor->y_block = (int)currentMeteor->y_pos;
                world[currentMeteor->x_block][currentMeteor->y_block][currentMeteor->z_block] = CUBE_METEOR;
            }
        }
        else{
            world[currentMeteor->x_block][currentMeteor->y_block][currentMeteor->z_block] = CUBE_METEOR;
        }

        currentMeteor = currentMeteor->next;
    }

    return(meteorList);
}


/*Bounce functions start here*/
void initiateBounce(){
    /*Fancy scenic teleport intitated by an aerial faith plate
      (in Layman's terms: "Bouncy thing make player go to other place")
    */

    /*y = (-((u - (PARABOLA_WIDTH))^2)) + BOUNCE_HEIGHT
      u is the horizontal (x and z combined) distance travelled from point a to b*/
}
