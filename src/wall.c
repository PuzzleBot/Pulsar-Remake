#include "graphics.h"


Wall z_walls[GRIDSIZE][GRIDSIZE-1];
Wall x_walls[GRIDSIZE-1][GRIDSIZE];
extern AnimationList * animationQueue;

extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);


/* Build the floor, static walls and pillars */
void buildStaticObjects(){
    int i, j, k;

    world[LEFTWALL][FLOORHEIGHT+3][BOTTOMWALL] = 3;

    /*Build the floor*/
    for(i = LEFTWALL; i <= RIGHTWALL; i++){
        for(j = BOTTOMWALL; j <= TOPWALL; j++){
            world[i][FLOORHEIGHT][j] = 4;
        }
    }

    /*Build the pillars*/
    for(i = LEFTWALL; i <= RIGHTWALL; i = i + ROOMSIZE + 1){
        for(j = BOTTOMWALL; j <= TOPWALL; j = j + ROOMSIZE + 1){
            for(k = 0; k <= WALL_HEIGHT; k++){
                world[i][FLOORHEIGHT+k][j] = 2;
            }
        }
    }

    if(DEBUG_MODE){
        for(i = LEFTWALL; i <= RIGHTWALL; i = i + ROOMSIZE + 5){
            for(j = BOTTOMWALL; j <= TOPWALL; j = j + ROOMSIZE + 1){
                for(k = 1; k <= WALL_HEIGHT; k++){
                    world[i][FLOORHEIGHT+k][j] = 3;
                }
            }
        }
    }

    /*Bottom and top borders*/
    for(i = LEFTWALL; i <= RIGHTWALL; i++){
        for(k = 1; k <= WALL_HEIGHT; k++){
            world[i][FLOORHEIGHT+k][BOTTOMWALL] = 2;
            world[i][FLOORHEIGHT+k][TOPWALL] = 2;
        }
    }

    /*Left and right borders*/
    for(j = BOTTOMWALL; j <= TOPWALL; j++){
        for(k = 1; k <= WALL_HEIGHT; k++){
            world[LEFTWALL][FLOORHEIGHT+k][j] = 2;
            world[RIGHTWALL][FLOORHEIGHT+k][j] = 2;
        }
    }
}


/* Adds in walls randomly at the start of the game */
void initializeWalls(){
    int chooseX;
    int chosenRow;
    int chosenCol;
    int closedWalls = 0;

    int worldX;
    int worldZ;

    int i, j;

    while(closedWalls < STARTING_WALLS){
        /*Do we choose from vertical or horizontal walls?*/
        chooseX = rand() % 2;
        if(chooseX == 1){
            /* Choose from x-alligned walls */
            chosenRow = (rand() % (GRIDSIZE-1));
            chosenCol = (rand() % GRIDSIZE);

            if(x_walls[chosenRow][chosenCol].state == OPEN){
                for(i = 1; i <= ROOMSIZE; i++){
                    worldX = LEFTWALL + (chosenCol * (ROOMSIZE+1)) + i;
                    worldZ = BOTTOMWALL + ((chosenRow + 1) * (ROOMSIZE+1));

                    /*"Build a wall" - Trump 2016*/
                    for(j = 1; j <= WALL_HEIGHT; j++){
                        world[worldX][FLOORHEIGHT+j][worldZ] = 6;
                    }
                }

                x_walls[chosenRow][chosenCol].state = CLOSED;
                closedWalls++;
            }
        }
        else{
            /* Choose from z-alligned walls */
            chosenRow = (rand() % GRIDSIZE);
            chosenCol = (rand() % (GRIDSIZE-1));

            if(z_walls[chosenRow][chosenCol].state == OPEN){
                for(i = 1; i <= ROOMSIZE; i++){
                    worldX = LEFTWALL + ((chosenCol + 1) * (ROOMSIZE+1));
                    worldZ = BOTTOMWALL + (chosenRow * (ROOMSIZE+1)) + i;

                    /*"Build a wall, and make the OS pay for it"*/
                    for(j = 1; j <= WALL_HEIGHT; j++){
                        world[worldX][FLOORHEIGHT+j][worldZ] = 6;
                    }
                }

                z_walls[chosenRow][chosenCol].state = CLOSED;
                closedWalls++;
            }
        }
    }

}

/*Initiates the sequence for opening/closing a wall*/
void toggleRandomWall(){
    int chooseX;
    int chosenRow;
    int chosenCol;
    int toggleable = 0;

    do{
        /*Do we choose from vertical or horizontal walls?*/
        chooseX = rand() % 2;
        if(chooseX == 1){
            /* Choose from x-alligned walls */
            chosenRow = (rand() % (GRIDSIZE-1));
            chosenCol = (rand() % GRIDSIZE);

            if((x_walls[chosenRow][chosenCol].state == OPENING) || (x_walls[chosenRow][chosenCol].state == CLOSING)){
                toggleable = 0;
            }
            else{
                toggleable = 1;
                if(x_walls[chosenRow][chosenCol].state == OPEN){
                    /*Close an open wall*/
                    addToAnimationQueue(CLOSING, 1, chosenRow, chosenCol);
                }
                else{
                    /*Open a closed wall*/
                    addToAnimationQueue(OPENING, 1, chosenRow, chosenCol);
                }
            }
        }
        else{
            /* Choose from z-alligned walls */
            chosenRow = (rand() % GRIDSIZE);
            chosenCol = (rand() % (GRIDSIZE-1));

            if((z_walls[chosenRow][chosenCol].state == OPENING) || (z_walls[chosenRow][chosenCol].state == CLOSING)){
                toggleable = 0;
            }
            else{
                toggleable = 1;
                if(z_walls[chosenRow][chosenCol].state == OPEN){
                    /*Close an open wall*/
                    addToAnimationQueue(CLOSING, 0, chosenRow, chosenCol);
                }
                else{
                    /*Open a closed wall*/
                    addToAnimationQueue(OPENING, 0, chosenRow, chosenCol);
                }
            }
        }
    }
    while(toggleable == 0);
}


/*Initiates the sequence for opening one wall and closing another wall at the same time*/
void toggleTwoWalls(){
    int chooseX;
    int chosenRow;
    int chosenCol;
    int toggleable = 0;

    do{
        /*Do we choose from vertical or horizontal walls?*/
        chooseX = rand() % 2;
        if(chooseX == 1){
            /* Choose from x-alligned walls */
            chosenRow = (rand() % (GRIDSIZE-1));
            chosenCol = (rand() % GRIDSIZE);

            if(x_walls[chosenRow][chosenCol].state != OPEN){
                toggleable = 0;
            }
            else{
                toggleable = 1;
                /*Close an open wall*/
                addToAnimationQueue(CLOSING, 1, chosenRow, chosenCol);
            }
        }
        else{
            /* Choose from z-alligned walls */
            chosenRow = (rand() % GRIDSIZE);
            chosenCol = (rand() % (GRIDSIZE-1));

            if(z_walls[chosenRow][chosenCol].state != OPEN){
                toggleable = 0;
            }
            else{
                toggleable = 1;
                /*Close an open wall*/
                addToAnimationQueue(CLOSING, 0, chosenRow, chosenCol);

            }
        }
    }
    while(toggleable == 0);

    toggleable = 0;

    do{
        /*Do we choose from vertical or horizontal walls?*/
        chooseX = rand() % 2;
        if(chooseX == 1){
            /* Choose from x-alligned walls */
            chosenRow = (rand() % (GRIDSIZE-1));
            chosenCol = (rand() % GRIDSIZE);

            if(x_walls[chosenRow][chosenCol].state != CLOSED){
                toggleable = 0;
            }
            else{
                toggleable = 1;
                /*Open a closed wall*/
                addToAnimationQueue(OPENING, 1, chosenRow, chosenCol);
            }
        }
        else{
            /* Choose from z-alligned walls */
            chosenRow = (rand() % GRIDSIZE);
            chosenCol = (rand() % (GRIDSIZE-1));

            if(z_walls[chosenRow][chosenCol].state != CLOSED){
                toggleable = 0;
            }
            else{
                toggleable = 1;
                /*Open a closed wall*/
                addToAnimationQueue(OPENING, 0, chosenRow, chosenCol);
            }
        }
    }
    while(toggleable == 0);
}

/*Adds a wall animation to the queue, to be executed on the subsequent updates until the animation
  has completed*/
void addToAnimationQueue(WallState animationType, Boolean isXwall, int row, int col){
    AnimationList * newAnimation = NULL;
    AnimationList * backOfQueue;

    newAnimation = malloc(sizeof(AnimationList));
    if(newAnimation != NULL){
        newAnimation->targetWallIndex[0] = row;
        newAnimation->targetWallIndex[1] = col;
        newAnimation->isXwall = isXwall;

        if(animationType == CLOSING){
            newAnimation->animationState = ROOMSIZE;
        }
        else{
            newAnimation->animationState = 1;
        }


        newAnimation->next = NULL;
        newAnimation->prev = NULL;

        /*Add to the back of the queue*/
        if(animationQueue == NULL){
            animationQueue = newAnimation;
        }
        else{
            backOfQueue = animationQueue;
            while(backOfQueue->next != NULL){
                backOfQueue = backOfQueue->next;
            }

            backOfQueue->next = newAnimation;
            newAnimation->prev = backOfQueue;
        }

        /*Initialize the animation state for the wall in the wall array*/
        if(isXwall == TRUE){
            x_walls[row][col].state = animationType;
        }
        else{
            z_walls[row][col].state = animationType;
        }
    }
    else{
        printf("Not enough memory!\n");
    }
}

/*Execute one step of each animation that is queued*/
void processAllAnimations(){
    AnimationList * currentAnimation = animationQueue;
    AnimationList * animBuffer;
    int worldX;
    int worldZ;
    int animationComplete = 0;

    float viewpointX;
    float viewpointY;
    float viewpointZ;

    Boolean mobInWall = FALSE;

    float roundedAmount;
    float roundedToValue;

    int i;

    /*Animate all walls in the middle of opening/closing*/
    while(currentAnimation != NULL){
        getViewPosition(&viewpointX, &viewpointY, &viewpointZ);
        viewpointX = -viewpointX;
        viewpointY = -viewpointY;
        viewpointZ = -viewpointZ;

        /*Update the block, decrement the animation state. If the state is zero, the animation
          is finished, and the animation should be removed from the queue.*/
        /*Calculate the location of each wall as needed*/
        if(currentAnimation->isXwall == 1){
            if(x_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state == CLOSING){
                /*Fill in the cubes*/
                worldX = LEFTWALL + (currentAnimation->targetWallIndex[1] * (ROOMSIZE+1)) + currentAnimation->animationState;
                worldZ = BOTTOMWALL + ((currentAnimation->targetWallIndex[0] + 1) * (ROOMSIZE+1));

                mobInWall = FALSE;
                for(i = 1; i <= WALL_HEIGHT; i++){
                    /*If there is a mob in the way, the wall "bounces back" (opens again)*/
                    if(world[worldX][FLOORHEIGHT+i][worldZ] != 0){
                        mobInWall = TRUE;
                    }
                }

                /*Nothing blocking the wall? Continue closing it*/
                if(mobInWall == FALSE){
                    for(i = 1; i <= WALL_HEIGHT; i++){
                        world[worldX][FLOORHEIGHT+i][worldZ] = 6;
                    }

                    /*If the player is inside the wall, push them out*/
                    if(((int)floor(viewpointX) == worldX) && ((int)floor(viewpointZ) == worldZ) &&
                       (((int)floor(viewpointY) == FLOORHEIGHT+1) || ((int)floor(viewpointY) == FLOORHEIGHT+2))){

                        roundedToValue = round(viewpointZ);
                        roundedAmount = roundedToValue - viewpointZ;
                        /*Round z, then push them away a bit. The rounded value minus the original value
                          determines which direction to push them.*/
                        if(roundedAmount >= 0){
                            /*Push up*/
                            setViewPosition(-viewpointX, -viewpointY, -(roundedToValue + 0.5));
                        }
                        else{
                            /*Push down*/
                            setViewPosition(-viewpointX, -viewpointY, -(roundedToValue - 0.5));
                        }
                    }
                    currentAnimation->animationState--;
                    if(currentAnimation->animationState <= 0){
                        x_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state = CLOSED;
                        animationComplete = 1;
                    }
                    else{
                        animationComplete = 0;
                    }
                }
                else{
                    /*There's a mob blocking the wall - reverse the wall closing*/
                    x_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state = OPENING;
                    if(currentAnimation->animationState > ROOMSIZE){
                        x_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state = OPEN;
                        animationComplete = 1;
                    }
                    else{
                        animationComplete = 0;
                    }
                }

            }
            else if(x_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state == OPENING){
                /*Remove the cubes*/
                worldX = LEFTWALL + (currentAnimation->targetWallIndex[1] * (ROOMSIZE+1)) + currentAnimation->animationState;
                worldZ = BOTTOMWALL + ((currentAnimation->targetWallIndex[0] + 1) * (ROOMSIZE+1));
                for(i = 1; i <= WALL_HEIGHT; i++){
                    world[worldX][FLOORHEIGHT+i][worldZ] = 0;
                }

                currentAnimation->animationState++;
                if(currentAnimation->animationState > ROOMSIZE){
                    x_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state = OPEN;
                    animationComplete = 1;
                }
                else{
                    animationComplete = 0;
                }
            }
        }
        else{
            if(z_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state == CLOSING){
                /*Fill in the cubes*/
                worldX = LEFTWALL + ((currentAnimation->targetWallIndex[1] + 1) * (ROOMSIZE+1));
                worldZ = BOTTOMWALL + (currentAnimation->targetWallIndex[0] * (ROOMSIZE+1)) + currentAnimation->animationState;

                mobInWall = FALSE;
                for(i = 1; i <= WALL_HEIGHT; i++){
                    /*If there is a mob in the way, the wall "bounces back" (opens again)*/
                    if(world[worldX][FLOORHEIGHT+i][worldZ] != 0){
                        mobInWall = TRUE;
                    }
                }

                /*Nothing blocking the wall? Continue closing it*/
                if(mobInWall == FALSE){
                    for(i = 1; i <= WALL_HEIGHT; i++){
                        world[worldX][FLOORHEIGHT+i][worldZ] = 6;
                    }

                    /*If the player is inside the wall, push them out*/
                    if(((int)floor(viewpointX) == worldX) && ((int)floor(viewpointZ) == worldZ) &&
                       (((int)floor(viewpointY) == FLOORHEIGHT+1) || ((int)floor(viewpointY) == FLOORHEIGHT+2))){

                        roundedToValue = round(viewpointX);
                        roundedAmount = roundedToValue - viewpointX;
                        /*Round x, then push them away a bit. The rounded value minus the original value
                         determines which direction to push them.*/
                        if(roundedAmount >= 0){
                            /*Push up*/
                            setViewPosition(-(roundedToValue + 0.5), -viewpointY, -viewpointZ);
                        }
                        else{
                            /*Push down*/
                            setViewPosition(-(roundedToValue - 0.5), -viewpointY, -viewpointZ);
                        }
                    }

                    currentAnimation->animationState--;
                    if(currentAnimation->animationState <= 0){
                        z_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state = CLOSED;
                        animationComplete = 1;
                    }
                    else{
                        animationComplete = 0;
                    }
                }
                else{
                    /*There's a mob blocking the wall - reverse the wall closing*/
                    z_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state = OPENING;
                    if(currentAnimation->animationState > ROOMSIZE){
                        z_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state = OPEN;
                        animationComplete = 1;
                    }
                    else{
                        animationComplete = 0;
                    }
                }
            }
            else if(z_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state == OPENING){
                /*Remove the cubes*/
                worldX = LEFTWALL + ((currentAnimation->targetWallIndex[1] + 1) * (ROOMSIZE+1));
                worldZ = BOTTOMWALL + (currentAnimation->targetWallIndex[0] * (ROOMSIZE+1)) + currentAnimation->animationState;
                for(i = 1; i <= WALL_HEIGHT; i++){
                    world[worldX][FLOORHEIGHT+i][worldZ] = 0;
                }

                currentAnimation->animationState++;
                if(currentAnimation->animationState > ROOMSIZE){
                    z_walls[currentAnimation->targetWallIndex[0]][currentAnimation->targetWallIndex[1]].state = OPEN;
                    animationComplete = 1;
                }
                else{
                    animationComplete = 0;
                }
            }
        }

        animBuffer = currentAnimation;
        currentAnimation = currentAnimation->next;
        if(animationComplete == 1){
            /*Remove completed animations from the queue*/
            deleteFromAnimationQueue(animBuffer);
        }
    }
}


/*Deletes an animation from the animation queue.*/
void deleteFromAnimationQueue(AnimationList * toDelete){
    if(toDelete == NULL){
        return;
    }


    if(toDelete->prev != NULL){
        toDelete->prev->next = toDelete->next;
    }

    if(toDelete->next != NULL){
        toDelete->next->prev = toDelete->prev;
    }

    if((toDelete->prev == NULL) && (toDelete->next == NULL)){
        animationQueue = NULL;
    }
    else if(toDelete->prev == NULL){
        animationQueue = toDelete->next;
    }
    /*else if(toDelete->next == NULL){
        animationQueue = toDelete->prev;
    }*/

    free(toDelete);
}


void printXWalls(){
    int i, j;

    for(i = 0; i < GRIDSIZE-1; i++){
        for(j = 0; j < GRIDSIZE; j++){
            if(x_walls[i][j].state == CLOSED){
                printf("-");
            }
            else{
                printf("O");
            }
        }
        printf("\n");
    }
}

void printZWalls(){
    int i, j;

    for(i = 0; i < GRIDSIZE; i++){
        for(j = 0; j < GRIDSIZE-1; j++){
            if(z_walls[i][j].state == CLOSED){
                printf("|");
            }
            else{
                printf("O");
            }
        }
        printf("\n");
    }
}
