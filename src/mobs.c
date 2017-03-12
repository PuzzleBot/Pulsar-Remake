#include "graphics.h"

extern HighGrid waypointGrid[(GRIDSIZE * 2) + 1][(GRIDSIZE * 2) + 1];
extern Bullet bulletArray[BULLET_ARRAY_SIZE];
Mob * mobList;

extern void getViewPosition(float *, float *, float *);
extern void getViewOrientation(float *, float *, float *);

Mob * createNewMob(MobType type, double x, double y, double z){
    int i, j, k;
    Mob * newMob = calloc(1, sizeof(Mob));

    if(newMob == NULL){
        printf("Error: Not enough memory!\n");
        exit(1);
    }

    newMob->type = type;

    newMob->x_pos = x;
    newMob->y_pos = y;
    newMob->z_pos = z;

    newMob->x_destinationBlock = -99;
    newMob->y_destinationBlock = -99;
    newMob->z_destinationBlock = -99;

    newMob->currentAiState = ROAMING;
    newMob->dodgeStepToggle = FALSE;
    newMob->currentHighGridCell[0] = 0;
    newMob->currentHighGridCell[1] = 1;

    /*Initialize the animation to a random frame for desync,
      fill in the mob's space to prevent collision*/
    switch(type){
        case ORBITER:
            newMob->currentAnimationFrame = rand() % 8;
            for(i = -1; i <= 1; i++){
                for(j = -1; j <= 1; j++){
                    for(k = -1; k <= 1; k++){
                        world[(int)x + i][(int)y + j][(int)z + k] = 8;
                    }
                }
            }
            break;
        case PULSAR:
            newMob->currentAnimationFrame = rand() % 4;
            for(i = -1; i <= 1; i++){
                for(j = -1; j <= 1; j++){
                    for(k = -1; k <= 1; k++){
                        world[(int)x + i][(int)y + j][(int)z + k] = 3;
                    }
                }
            }
            break;
        default:
            break;
    }

    newMob->next = NULL;
    newMob->prev = NULL;

    return(newMob);
}

void generateValidMobPosition(int * x, int * y, int * z){
    /*-2: Prevent the mob from spawning on the right of or above the outer border*/
    static int horizRange = RIGHTWALL - LEFTWALL - 3;
    static int vertRange = TOPWALL - BOTTOMWALL - 3;
    static int mobHeight = FLOORHEIGHT + 2.2;
    int randomX;
    int randomZ;
    Boolean collisionDetected;
    int i, j;

    do{
        randomX = rand() % horizRange;
        randomZ = rand() % vertRange;
        /*Prevent spawning in the starting cell unless it is the only one*/
        if(((randomX >= (RIGHTWALL - ROOMSIZE)) && (randomZ <= ROOMSIZE)) && (GRIDSIZE > 1)){
            /*Fix it by offsetting x and z to the northeast cell*/
            randomX = randomX + (ROOMSIZE + 1);
            randomZ = randomZ + (ROOMSIZE + 1);
        }

        /*Avoid spawing the mob inside a wall
          +1: W...... -> .W.....*/
        if(((randomX + 1) % (ROOMSIZE+1)) <= 2){
            randomX = randomX + 3;
        }

        if(((randomZ + 1) % (ROOMSIZE+1)) <= 2){
            randomZ = randomZ + 3;
        }

        collisionDetected = FALSE;
        for(i = -1; i <= 1; i++){
            for(j = -1; j <= 1; j++){
                if(world[LEFTWALL + randomX + i][mobHeight][BOTTOMWALL + randomZ + j] != 0){
                    collisionDetected = TRUE;
                }
            }
        }
    } while(collisionDetected == TRUE);

    *x = LEFTWALL + randomX;
    *y = mobHeight;
    *z = BOTTOMWALL + randomZ;
}

/*Linked list add to front - returns the head of the list*/
Mob * mobAddToFront(Mob * mobList, Mob * newMob){
    if(mobList == NULL){
        mobList = newMob;
        return mobList;
    }
    else{
        newMob->next = mobList;
        mobList->prev = newMob;
        return newMob;
    }
}

/*Linked list delete from front - returns the new head of the list*/
Mob * mobDeleteFromFront(Mob * mobList){
    Mob * newHead = mobList->next;

    if(mobList->next != NULL){
        mobList->next->prev = NULL;
    }

    free(mobList);
    return newHead;
}

Mob * generateRandomMobs(int count){
    int i;
    MobType chosenType = ORBITER;
    Mob * newMobList = NULL;

    int mobX, mobY, mobZ;

    for(i = 0; i < count; i++){
        /*Create at least one of each mob type*/
        if(i == 0){
            chosenType = ORBITER;
        }
        else if(i == 1){
            chosenType = PULSAR;
        }
        else{
            switch(rand() % 2){
                case 0:
                    chosenType = ORBITER;
                    break;
                case 1:
                    chosenType = PULSAR;
                    break;
                default:
                    chosenType = PULSAR;
                    break;
            }
        }

        generateValidMobPosition(&mobX, &mobY, &mobZ);
        newMobList = mobAddToFront(newMobList, createNewMob(chosenType, mobX, mobY, mobZ));

        newMobList->mobBullet = &bulletArray[MOB_BULLET_ARRAY_START + i];
        newMobList->bulletArrayId = MOB_BULLET_ARRAY_START + i;
    }

    return(newMobList);
}

int checkMobCollision(double destinationX, double destinationY, double destinationZ, Mob * mob){
    int i, j, k;
    int collisionColour = 0;

    for(i = -1; i <= 1; i++){
        for(j = -1; j <= 1; j++){
            for(k = -1; k <= 1; k++){
                if((abs((int)destinationX + i - (int)mob->x_pos) > 1) || (abs((int)destinationY + j - (int)mob->y_pos) > 1) || (abs((int)destinationZ + k - (int)mob->z_pos) > 1)){
                    if(world[(int)destinationX + i][(int)destinationY + j][(int)destinationZ + k] != 0){
                        if((collisionColour != 2) && (collisionColour != 6)){
                            collisionColour = world[(int)destinationX + i][(int)destinationY + j][(int)destinationZ + k];
                        }
                    }
                }
            }
        }
    }

    return collisionColour;
}

void clearMobSpace(int coreX, int coreY, int coreZ){
    int i, j, k;

    for(i = -1; i <= 1; i++){
        for(j = -1; j <= 1; j++){
            for(k = -1; k <= 1; k++){
                world[coreX + i][coreY + j][coreZ + k] = 0;
            }
        }
    }
}

void animateSingleMob(Mob * mob){
    int i, j, k;
    int mobBlockX = (int)mob->x_pos;
    int mobBlockY = (int)mob->y_pos;
    int mobBlockZ = (int)mob->z_pos;

    for(i = -1; i <= 1; i++){
        for(j = -1; j <= 1; j++){
            for(k = -1; k <= 1; k++){
                world[mobBlockX + i][mobBlockY + j][mobBlockZ + k] = 0;
            }
        }
    }

    /*Movement*/

    if(mob->type == ORBITER){
        /*Static parts of the mob*/
        world[mobBlockX][mobBlockY][mobBlockZ] = 8;
        world[mobBlockX+1][mobBlockY][mobBlockZ] = 8;
        world[mobBlockX-1][mobBlockY][mobBlockZ] = 8;
        world[mobBlockX][mobBlockY+1][mobBlockZ] = 8;
        world[mobBlockX][mobBlockY-1][mobBlockZ] = 8;
        world[mobBlockX][mobBlockY][mobBlockZ+1] = 8;
        world[mobBlockX][mobBlockY][mobBlockZ-1] = 8;

        /*Animated pieces of the mob*/
        switch(mob->currentAnimationFrame){
            case 0:
                world[mobBlockX+1][mobBlockY][mobBlockZ] = 7;
                break;
            case 1:
                world[mobBlockX+1][mobBlockY][mobBlockZ+1] = 7;
                break;
            case 2:
                world[mobBlockX][mobBlockY][mobBlockZ+1] = 7;
                break;
            case 3:
                world[mobBlockX-1][mobBlockY][mobBlockZ+1] = 7;
                break;
            case 4:
                world[mobBlockX-1][mobBlockY][mobBlockZ] = 7;
                break;
            case 5:
                world[mobBlockX-1][mobBlockY][mobBlockZ-1] = 7;
                break;
            case 6:
                world[mobBlockX][mobBlockY][mobBlockZ-1] = 7;
                break;
            case 7:
                world[mobBlockX+1][mobBlockY][mobBlockZ-1] = 7;
                break;
            default:
                break;
        }
        mob->currentAnimationFrame = (mob->currentAnimationFrame + 1) % 8;
    }
    else if(mob->type == PULSAR){
        /*Static parts of the mob*/
        world[mobBlockX][mobBlockY][mobBlockZ] = 3;
        world[mobBlockX+1][mobBlockY+1][mobBlockZ+1] = 3;
        world[mobBlockX+1][mobBlockY+1][mobBlockZ-1] = 3;
        world[mobBlockX-1][mobBlockY+1][mobBlockZ+1] = 3;
        world[mobBlockX-1][mobBlockY+1][mobBlockZ-1] = 3;
        world[mobBlockX+1][mobBlockY-1][mobBlockZ+1] = 3;
        world[mobBlockX+1][mobBlockY-1][mobBlockZ-1] = 3;
        world[mobBlockX-1][mobBlockY-1][mobBlockZ+1] = 3;
        world[mobBlockX-1][mobBlockY-1][mobBlockZ-1] = 3;

        /*Animated pieces of the mob*/
        switch(mob->currentAnimationFrame){
            case 0:
                break;
            case 1:
                world[mobBlockX+1][mobBlockY][mobBlockZ] = 3;
                world[mobBlockX-1][mobBlockY][mobBlockZ] = 3;
                world[mobBlockX][mobBlockY+1][mobBlockZ] = 3;
                world[mobBlockX][mobBlockY-1][mobBlockZ] = 3;
                world[mobBlockX][mobBlockY][mobBlockZ+1] = 3;
                world[mobBlockX][mobBlockY][mobBlockZ-1] = 3;
                break;
            case 2:
                world[mobBlockX+1][mobBlockY][mobBlockZ] = 3;
                world[mobBlockX-1][mobBlockY][mobBlockZ] = 3;
                world[mobBlockX][mobBlockY+1][mobBlockZ] = 3;
                world[mobBlockX][mobBlockY-1][mobBlockZ] = 3;
                world[mobBlockX][mobBlockY][mobBlockZ+1] = 3;
                world[mobBlockX][mobBlockY][mobBlockZ-1] = 3;

                world[mobBlockX+1][mobBlockY+1][mobBlockZ] = 3;
                world[mobBlockX+1][mobBlockY-1][mobBlockZ] = 3;
                world[mobBlockX+1][mobBlockY][mobBlockZ+1] = 3;
                world[mobBlockX+1][mobBlockY][mobBlockZ-1] = 3;
                world[mobBlockX][mobBlockY+1][mobBlockZ+1] = 3;
                world[mobBlockX][mobBlockY-1][mobBlockZ+1] = 3;
                world[mobBlockX][mobBlockY+1][mobBlockZ-1] = 3;
                world[mobBlockX][mobBlockY-1][mobBlockZ-1] = 3;
                world[mobBlockX-1][mobBlockY+1][mobBlockZ] = 3;
                world[mobBlockX-1][mobBlockY-1][mobBlockZ] = 3;
                world[mobBlockX-1][mobBlockY][mobBlockZ+1] = 3;
                world[mobBlockX-1][mobBlockY][mobBlockZ-1] = 3;
                break;
            case 3:
                world[mobBlockX+1][mobBlockY][mobBlockZ] = 3;
                world[mobBlockX-1][mobBlockY][mobBlockZ] = 3;
                world[mobBlockX][mobBlockY+1][mobBlockZ] = 3;
                world[mobBlockX][mobBlockY-1][mobBlockZ] = 3;
                world[mobBlockX][mobBlockY][mobBlockZ+1] = 3;
                world[mobBlockX][mobBlockY][mobBlockZ-1] = 3;
                break;
            default:
                break;
        }
        mob->currentAnimationFrame = (mob->currentAnimationFrame + 1) % 4;
    }
}

void animateAllMobs(){
    Mob * currentMob = mobList;
    while(currentMob != NULL){
        processMobAI(currentMob);
        animateSingleMob(currentMob);
        currentMob = currentMob->next;
    }
}

void processMobAI(Mob * mob){
    BlockList * blocksInPath = NULL;
    int mobHighGridCellX;
    int mobHighGridCellY;

    float playerX, playerY, playerZ;

    int numberOfOpenPaths = 0;
    Direction openDirections[4];
    int cellsToTraverse = 1;

    /*Detect line of sight, possibly shoot a projectile, change ai state accordingly*/
    getViewPosition(&playerX, &playerY, &playerZ);
    playerX = -playerX;
    playerY = -playerY;
    playerZ = -playerZ;

    blocksInPath = getAllBlocksOnLine(blocksInPath, mob->x_pos, mob->y_pos, mob->z_pos, (double)playerX, (double)playerY, (double)playerZ);

    if(detectWallInPath(blocksInPath) == FALSE){
        /*Line of sight from mob to player*/
        fireBulletFromMobToPlayer(mob, playerX, playerY, playerZ);

        if(checkForPlayerWatching(mob, playerX, playerY, playerZ) == TRUE){
            mob->currentAiState = DODGING;
        }
        else{
            mob->currentAiState = ROAMING;
        }
    }
    else{
        mob->currentAiState = ROAMING;
    }

    deleteBlockList(blocksInPath);
    blocksInPath = NULL;

    /*Move depending on state*/
    if(mob->currentAiState == ROAMING){
        /*If the mob is sitting still, start moving towards another cell*/
        if(mobHasObjective(mob) == FALSE){
            /*Look for a surrounding cell that is not blocked by a wall*/
            /*Left path*/
            if(lineIsClear(mob->x_pos, mob->y_pos, mob->z_pos, mob->x_pos - fmod((mob->x_pos - (double)LEFTWALL), (GRIDSIZE+1)) - (ROOMSIZE/2), mob->y_pos, mob->z_pos) == TRUE){
                openDirections[numberOfOpenPaths] = LEFT;
                numberOfOpenPaths++;
                //printf("Left ");
            }

            /*Right path*/
            if(lineIsClear(mob->x_pos, mob->y_pos, mob->z_pos, mob->x_pos + (ROOMSIZE+1 - fmod((mob->x_pos - (double)LEFTWALL), (GRIDSIZE+1))) + (ROOMSIZE/2), mob->y_pos, mob->z_pos) == TRUE){
                openDirections[numberOfOpenPaths] = RIGHT;
                numberOfOpenPaths++;
                //printf("Right ");
            }

            /*Lower path*/
            if(lineIsClear(mob->x_pos, mob->y_pos, mob->z_pos, mob->x_pos, mob->y_pos, mob->z_pos - fmod((mob->z_pos - (double)BOTTOMWALL), (GRIDSIZE+1)) - (ROOMSIZE/2)) == TRUE){
                openDirections[numberOfOpenPaths] = DOWN;
                numberOfOpenPaths++;
                //printf("Down ");
            }

            /*Upper path*/
            if(lineIsClear(mob->x_pos, mob->y_pos, mob->z_pos, mob->x_pos, mob->y_pos, mob->z_pos + (ROOMSIZE+1 - fmod((mob->z_pos - (double)BOTTOMWALL), (GRIDSIZE+1))) + (ROOMSIZE/2)) == TRUE){
                openDirections[numberOfOpenPaths] = UP;
                numberOfOpenPaths++;
                //printf("Up ");
            }


            /*Choose a path at random*/
            if(numberOfOpenPaths > 0){
                switch(openDirections[rand() % numberOfOpenPaths]){
                    case LEFT:
                        mob->x_velocity = -MOB_MOVEMENT_SPEED;
                        mob->y_velocity = 0;
                        mob->z_velocity = 0;

                        mob->x_destinationBlock = (int)mob->x_pos - (((int)mob->x_pos - LEFTWALL) % (ROOMSIZE+1)) - (ROOMSIZE/2);
                        mob->y_destinationBlock = (int)mob->y_pos;
                        mob->z_destinationBlock = (int)mob->z_pos;
                        //printf(", Choose left\n");
                        break;
                    case RIGHT:
                        mob->x_velocity = MOB_MOVEMENT_SPEED;
                        mob->y_velocity = 0;
                        mob->z_velocity = 0;

                        mob->x_destinationBlock = (int)mob->x_pos + (ROOMSIZE+1 - (((int)mob->x_pos - LEFTWALL) % (ROOMSIZE+1))) + (ROOMSIZE/2);
                        mob->y_destinationBlock = (int)mob->y_pos;
                        mob->z_destinationBlock = (int)mob->z_pos;
                        //printf(", Choose right\n");
                        break;
                    case DOWN:
                        mob->x_velocity = 0;
                        mob->y_velocity = 0;
                        mob->z_velocity = -MOB_MOVEMENT_SPEED;

                        mob->x_destinationBlock = (int)mob->x_pos;
                        mob->y_destinationBlock = (int)mob->y_pos;
                        mob->z_destinationBlock = (int)mob->z_pos - (((int)mob->z_pos - BOTTOMWALL) % (ROOMSIZE+1)) - (ROOMSIZE/2);
                        //printf(", Choose down\n");
                        break;
                    case UP:
                        mob->x_velocity = 0;
                        mob->y_velocity = 0;
                        mob->z_velocity = MOB_MOVEMENT_SPEED;

                        mob->x_destinationBlock = (int)mob->x_pos;
                        mob->y_destinationBlock = (int)mob->y_pos;
                        mob->z_destinationBlock = (int)mob->z_pos + (ROOMSIZE+1 - (((int)mob->z_pos - BOTTOMWALL) % (ROOMSIZE+1))) + (ROOMSIZE/2);
                        //printf(", Choose up\n");
                        break;
                    default:
                        printf("Error\n");
                        break;
                }
                world[mob->x_destinationBlock][FLOORHEIGHT][mob->z_destinationBlock] = 5;
            }
            else{
                discardMobObjective(mob);
            }
        }
        else{
            /*Currently moving, check if a wall closes in the middle of a path
              or the cell has been reached (note: no out of bounds check needed here)*/

            printf("At %d %d\n", (int)mob->x_pos, (int)mob->z_pos);
            printf("Velocity %.2f %.2f\n", mob->x_velocity, mob->z_velocity);
            printf("Moving to: %d %d\n", mob->x_destinationBlock, mob->z_destinationBlock);

            if(mob->x_velocity < -0.001){
                /*Leftward movement - left wall check*/
                /*Is the mob at the x-position it wanted to travel to?*/
                if((int)mob->x_pos < mob->x_destinationBlock){
                    /*Destination reached*/
                    discardMobObjective(mob);
                }
                else{
                    /*Destination ahead - check for a wall blocking the path*/
                    if((lineIsClear(mob->x_pos+1, mob->y_pos, mob->z_pos, mob->x_destinationBlock, mob->y_destinationBlock, mob->z_destinationBlock) == FALSE)
                        || (lineIsClear(mob->x_pos+1, mob->y_pos+1, mob->z_pos+1, mob->x_destinationBlock, mob->y_destinationBlock+1, mob->z_destinationBlock+1) == FALSE)
                        || (lineIsClear(mob->x_pos+1, mob->y_pos-1, mob->z_pos-1, mob->x_destinationBlock, mob->y_destinationBlock-1, mob->z_destinationBlock-1) == FALSE)){
                        discardMobObjective(mob);
                    }
                }
            }
            else if(mob->x_velocity > 0.001){
                /*Rightward movement - right wall check*/
                /*Is the mob at the cell it wanted to travel to?*/
                if((int)mob->x_pos > mob->x_destinationBlock){
                    /*Destination reached*/
                    discardMobObjective(mob);
                }
                else{
                    /*Destination ahead - check for a wall blocking the path*/
                    if((lineIsClear(mob->x_pos-1, mob->y_pos, mob->z_pos, mob->x_destinationBlock, mob->y_destinationBlock, mob->z_destinationBlock) == FALSE)
                        || (lineIsClear(mob->x_pos-1, mob->y_pos+1, mob->z_pos+1, mob->x_destinationBlock, mob->y_destinationBlock+1, mob->z_destinationBlock+1) == FALSE)
                        || (lineIsClear(mob->x_pos-1, mob->y_pos-1, mob->z_pos-1, mob->x_destinationBlock, mob->y_destinationBlock-1, mob->z_destinationBlock-1) == FALSE)){
                        discardMobObjective(mob);
                    }
                }
            }
            else if(mob->z_velocity < -0.001){
                /*Downward movement - south wall check*/
                /*Is the mob at the cell it wanted to travel to?*/
                if((int)mob->z_pos < mob->z_destinationBlock){
                    /*Destination reached*/
                    discardMobObjective(mob);
                }
                else{
                    /*Destination ahead - check for a wall blocking the path*/
                    if((lineIsClear(mob->x_pos, mob->y_pos, mob->z_pos+1, mob->x_destinationBlock, mob->y_destinationBlock, mob->z_destinationBlock) == FALSE)
                        || (lineIsClear(mob->x_pos+1, mob->y_pos+1, mob->z_pos+1, mob->x_destinationBlock+1, mob->y_destinationBlock+1, mob->z_destinationBlock) == FALSE)
                        || (lineIsClear(mob->x_pos-1, mob->y_pos-1, mob->z_pos+1, mob->x_destinationBlock+1, mob->y_destinationBlock-1, mob->z_destinationBlock) == FALSE)){
                        discardMobObjective(mob);
                    }
                }
            }
            else if(mob->z_velocity > 0.001){
                /*Upward movement - north wall check*/
                /*Is the mob at the cell it wanted to travel to?*/
                if((int)mob->z_pos > mob->z_destinationBlock){
                    /*Destination reached*/
                    discardMobObjective(mob);
                }
                else{
                    /*Destination ahead - check for a wall blocking the path*/
                    if((lineIsClear(mob->x_pos, mob->y_pos, mob->z_pos-1, mob->x_destinationBlock, mob->y_destinationBlock, mob->z_destinationBlock) == FALSE)
                        || (lineIsClear(mob->x_pos+1, mob->y_pos+1, mob->z_pos-1, mob->x_destinationBlock+1, mob->y_destinationBlock+1, mob->z_destinationBlock) == FALSE)
                        || (lineIsClear(mob->x_pos-1, mob->y_pos-1, mob->z_pos-1, mob->x_destinationBlock+1, mob->y_destinationBlock-1, mob->z_destinationBlock) == FALSE)){
                        discardMobObjective(mob);
                    }
                }
            }
        }

        if(mobHasObjective(mob) == TRUE){
            moveMob(mob);
        }
    }
    else if(mob->currentAiState == DODGING){
        /*Do nothing*/
    }

    /*Velocity not zero - move the mob*/
    //printf("Velocity: %.2f %.2f %.2f\n", fabs(mob->x_velocity), fabs(mob->y_velocity), fabs(mob->z_velocity));
}

Boolean moveMob(Mob * mob){
    double newX_pos = mob->x_pos + mob->x_velocity;
    double newY_pos = mob->y_pos + mob->y_velocity;
    double newZ_pos = mob->z_pos + mob->z_velocity;

    int collidedBlock = 0;

    /*Mob is already at its destination - do nothing*/
    if(((int)newX_pos == (int)mob->x_pos) && ((int)newY_pos == (int)mob->y_pos) && ((int)newZ_pos == (int)mob->z_pos)){
        clearMobSpace((int)mob->x_pos, (int)mob->y_pos, (int)mob->z_pos);
        mob->x_pos = newX_pos;
        mob->z_pos = newZ_pos;
        return TRUE;
    }
    else{
        collidedBlock = checkMobCollision(newX_pos, newY_pos, newZ_pos, mob);
        if(collidedBlock == 0){
            /*No collision - safe to move the mob*/
            clearMobSpace((int)mob->x_pos, (int)mob->y_pos, (int)mob->z_pos);
            mob->x_pos = newX_pos;
            mob->z_pos = newZ_pos;
            return TRUE;
        }
        else{
            /*Sidestep by rotating current velocity by 90 degrees counterclockwise
              to try to get around the obstacle*/
            //printf("Attempting to sidestep\n");
            newX_pos = mob->x_pos + mob->z_velocity;
            newZ_pos = mob->z_pos + mob->x_velocity;
            if(checkMobCollision(newX_pos, newY_pos, newZ_pos, mob) == 0){
                clearMobSpace((int)mob->x_pos, (int)mob->y_pos, (int)mob->z_pos);
                mob->x_pos = newX_pos;
                mob->z_pos = newZ_pos;
            }
            else{
                /*If the mob can't sidestep that way, try the other way*/
                newX_pos = -newX_pos;
                newZ_pos = -newZ_pos;
                if(checkMobCollision(newX_pos, newY_pos, newZ_pos, mob) == 0){
                    clearMobSpace((int)mob->x_pos, (int)mob->y_pos, (int)mob->z_pos);
                    mob->x_pos = newX_pos;
                    mob->z_pos = newZ_pos;
                }
                else{
                    /*Can't sidestep - don't do anything, wait for the other AI to hopefully move*/
                    discardMobObjective(mob);
                }
            }
            return FALSE;
        }
    }
}

void stopMob(Mob * mob){
    mob->x_velocity = 0;
    mob->y_velocity = 0;
    mob->z_velocity = 0;
}

void discardMobObjective(Mob * mob){
    world[mob->x_destinationBlock][FLOORHEIGHT][mob->z_destinationBlock] = 4;
    mob->x_destinationBlock = -99;
    mob->y_destinationBlock = -99;
    mob->z_destinationBlock = -99;
    stopMob(mob);
}

Boolean mobHasObjective(Mob * mob){
    if((mob->x_destinationBlock <= -1) || (mob->y_destinationBlock <= -1) || (mob->z_destinationBlock <= -1)){
        return FALSE;
    }
    else{
        return TRUE;
    }
}


Boolean checkForPlayerWatching(Mob * mob, double vpX, double vpY, double vpZ){
    double lookDirectionX, lookDirectionY, lookDirectionZ;
    float vpOrientX, vpOrientY, vpOrientZ;
    double lookDirectionLength;
    double mobVpX = mob->x_pos - vpX;
    double mobVpY = mob->y_pos - vpY;
    double mobVpZ = mob->z_pos - vpZ;
    double mobVpLength = sqrt(pow(mobVpX, 2) + pow(mobVpY, 2) + pow(mobVpZ, 2));

    double angleBetweenLines;
    static int count = 0;

    getViewOrientation(&vpOrientX, &vpOrientY, &vpOrientZ);
    vpOrientX = -vpOrientX;
    vpOrientY = -vpOrientY;
    vpOrientZ = -vpOrientZ;

    vpOrientX = fmod(vpOrientX, 360.0) * (M_PI / 180.0);
    vpOrientY = fmod(vpOrientY, 360.0) * (M_PI / 180.0);
    vpOrientZ = fmod(vpOrientZ, 360.0) * (M_PI / 180.0);

    lookDirectionX = -sin(vpOrientY);
    lookDirectionY = 0;
    lookDirectionZ = -cos(vpOrientY);

    /*Normalize the direction*/
    lookDirectionLength = sqrt(pow(lookDirectionX, 2) + pow(lookDirectionY, 2) + pow(lookDirectionZ, 2));
    lookDirectionX = lookDirectionX / lookDirectionLength;
    lookDirectionY = lookDirectionY / lookDirectionLength;
    lookDirectionZ = lookDirectionZ / lookDirectionLength;

    mobVpX = mobVpX / mobVpLength;
    mobVpY = mobVpY / mobVpLength;
    mobVpZ = mobVpZ / mobVpLength;

    angleBetweenLines = acos(dotProduct2D(lookDirectionX, lookDirectionZ, mobVpX, mobVpZ));

    if(angleBetweenLines < (M_PI / 8)){
        //count++;
        //printf("Looking %d\n", count);
        dodgeLineOfSight(mob, lookDirectionX, lookDirectionY, lookDirectionZ);
        return TRUE;
    }
    else{
        return FALSE;
    }
}


Boolean dodgeLineOfSight(Mob * mob, double lookX, double lookY, double lookZ){
    double newX_pos, newZ_pos;
    double perpendicularX, perpendicularZ;
    double perpendicularLength;
    int collidedBlock = 0;

    /*Perpendicular vector*/
    perpendicularX = -lookZ;
    perpendicularZ = lookX;

    perpendicularLength = sqrt((lookX * lookX) + (lookZ * lookZ));

    /*For reversing direction*/
    if(mob->dodgeStepToggle == FALSE){
        newX_pos = mob->x_pos + (MOB_MOVEMENT_SPEED * (perpendicularX / perpendicularLength));
        newZ_pos = mob->z_pos + (MOB_MOVEMENT_SPEED * (perpendicularZ / perpendicularLength));
    }
    else{
        newX_pos = mob->x_pos - (MOB_MOVEMENT_SPEED * (perpendicularX / perpendicularLength));
        newZ_pos = mob->z_pos - (MOB_MOVEMENT_SPEED * (perpendicularZ / perpendicularLength));
    }

    /*Mob is already at its destination - do nothing*/
    if(((int)newX_pos == (int)mob->x_pos) && ((int)newZ_pos == (int)mob->z_pos)){
        clearMobSpace((int)mob->x_pos, (int)mob->y_pos, (int)mob->z_pos);
        //printf("Move: %.2f %.2f\n", (MOB_MOVEMENT_SPEED * (perpendicularX / perpendicularLength)), (MOB_MOVEMENT_SPEED * (perpendicularZ / perpendicularLength)));
        mob->x_pos = newX_pos;
        mob->z_pos = newZ_pos;
        return TRUE;
    }
    else{
        collidedBlock = checkMobCollision(newX_pos, mob->y_pos, newZ_pos, mob);
        if(collidedBlock == 0){
            /*No collision - safe to sidestep the mob*/
            clearMobSpace((int)mob->x_pos, (int)mob->y_pos, (int)mob->z_pos);
            mob->x_pos = newX_pos;
            mob->z_pos = newZ_pos;
            return TRUE;
        }
        else{
            /*Collision - try going the other way*/
            if(mob->dodgeStepToggle == TRUE){
                newX_pos = mob->x_pos + (MOB_MOVEMENT_SPEED * (perpendicularX / perpendicularLength));
                newZ_pos = mob->z_pos + (MOB_MOVEMENT_SPEED * (perpendicularZ / perpendicularLength));
            }
            else{
                newX_pos = mob->x_pos - (MOB_MOVEMENT_SPEED * (perpendicularX / perpendicularLength));
                newZ_pos = mob->z_pos - (MOB_MOVEMENT_SPEED * (perpendicularZ / perpendicularLength));
            }

            if(checkMobCollision(newX_pos, mob->y_pos, newZ_pos, mob) == 0){
                clearMobSpace((int)mob->x_pos, (int)mob->y_pos, (int)mob->z_pos);
                if(mob->dodgeStepToggle == FALSE){ mob->dodgeStepToggle = TRUE; }
                else { mob->dodgeStepToggle = FALSE; }
                mob->x_pos = newX_pos;
                mob->z_pos = newZ_pos;
            }
            else{
                /*If the mob can't sidestep at all, don't move*/
                clearMobSpace((int)mob->x_pos, (int)mob->y_pos, (int)mob->z_pos);
            }
            return FALSE;
        }
    }
}

double dotProduct2D(double x1, double z1, double x2, double z2){
    return((x1 * x2) + (z1 * z2));
}


void worldMobInit(){
    mobList = generateRandomMobs(MOB_SPAWN);
}
