#include "graphics.h"

extern HighGrid waypointGrid[(GRIDSIZE * 2) + 1][(GRIDSIZE * 2) + 1];
extern Bullet bulletArray[BULLET_ARRAY_SIZE];
Mob * mobList;

extern void getViewPosition(float *, float *, float *);

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

    newMob->currentAiState = ROAMING;
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
    static int mobHeight = FLOORHEIGHT + 2.1;
    int randomX;
    int randomZ;
    Boolean collisionDetected;
    int i, j;

    do{
        randomX = rand() % horizRange;
        randomZ = rand() % vertRange;
        /*Prevent spawning in the starting cell unless it is the only one*/
        if(((randomX <= ROOMSIZE) && (randomZ <= ROOMSIZE)) && (GRIDSIZE > 1)){
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
    }

    deleteBlockList(blocksInPath);
    blocksInPath = NULL;

    return;

    /*Move depending on state*/
    if(mob->currentAiState == ROAMING){
        /*If the mob is sitting still, start moving towards another cell*/
        if((fabs(mob->x_velocity) <= 0.01) && (fabs(mob->y_velocity) <= 0.01) && (fabs(mob->z_velocity) <= 0.01)){
            /*Look for a surrounding cell that is not blocked by a wall*/
            getCorrespondingHighGridIndex(&mobHighGridCellX, &mobHighGridCellY, (int)mob->x_pos, (int)mob->z_pos);
            if(waypointGrid[mobHighGridCellY][mobHighGridCellX].type != HIGHGRID_CELL){
                printf("WALL! %d - %d %d\n", waypointGrid[mobHighGridCellY][mobHighGridCellX].type, mobHighGridCellX, mobHighGridCellY);
                /*Mob is in the path of a wall - move into an adjacent cell*/
                if(mobHighGridCellX % (ROOMSIZE+1) == 0){
                    /*In the path of an x-alligned wall (horizontal) - move left/right*/
                    printf("Horizontal\n");
                    openDirections[numberOfOpenPaths] = LEFT;
                    numberOfOpenPaths++;
                    openDirections[numberOfOpenPaths] = RIGHT;
                    numberOfOpenPaths++;
                }

                if(mobHighGridCellY % (ROOMSIZE+1) == 0){
                    /*In the path of an z-alligned wall (vertical) - move up/down*/
                    printf("Vertical\n");
                    openDirections[numberOfOpenPaths] = UP;
                    numberOfOpenPaths++;
                    openDirections[numberOfOpenPaths] = DOWN;
                    numberOfOpenPaths++;
                }

                cellsToTraverse = 1;
            }
            else{
                /*Left path*/
                if((mobHighGridCellX > 0) && (waypointGrid[mobHighGridCellY][mobHighGridCellX - 1].correspondingWall != NULL) && (waypointGrid[mobHighGridCellY][mobHighGridCellX - 1].correspondingWall->state == OPEN)){
                    openDirections[numberOfOpenPaths] = LEFT;
                    numberOfOpenPaths++;
                }

                /*Right path*/
                if((mobHighGridCellX < (GRIDSIZE * 2) + 1) && (waypointGrid[mobHighGridCellY][mobHighGridCellX + 1].correspondingWall != NULL) && (waypointGrid[mobHighGridCellY][mobHighGridCellX + 1].correspondingWall->state == OPEN)){
                    openDirections[numberOfOpenPaths] = RIGHT;
                    numberOfOpenPaths++;
                }

                /*Lower path*/
                if((mobHighGridCellY > 0) && (waypointGrid[mobHighGridCellY - 1][mobHighGridCellX].correspondingWall != NULL) && (waypointGrid[mobHighGridCellY - 1][mobHighGridCellX].correspondingWall->state == OPEN)){
                    openDirections[numberOfOpenPaths] = DOWN;
                    numberOfOpenPaths++;
                }

                /*Upper path*/
                if((mobHighGridCellY < (GRIDSIZE * 2) + 1) && (waypointGrid[mobHighGridCellY + 1][mobHighGridCellX].correspondingWall != NULL) && (waypointGrid[mobHighGridCellY + 1][mobHighGridCellX].correspondingWall->state == OPEN)){
                    openDirections[numberOfOpenPaths] = UP;
                    numberOfOpenPaths++;
                }

                cellsToTraverse = 2;
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
                        break;
                    case RIGHT:
                        mob->x_velocity = MOB_MOVEMENT_SPEED;
                        mob->y_velocity = 0;
                        mob->z_velocity = 0;

                        mob->x_destinationBlock = (int)mob->x_pos + (ROOMSIZE+1 - (((int)mob->x_pos - LEFTWALL) % (ROOMSIZE+1))) + (ROOMSIZE/2);
                        mob->y_destinationBlock = (int)mob->y_pos;
                        mob->z_destinationBlock = (int)mob->z_pos;
                        break;
                    case UP:
                        mob->x_velocity = 0;
                        mob->y_velocity = 0;
                        mob->z_velocity = MOB_MOVEMENT_SPEED;

                        mob->x_destinationBlock = (int)mob->x_pos;
                        mob->y_destinationBlock = (int)mob->y_pos;
                        mob->z_destinationBlock = (int)mob->z_pos - (((int)mob->z_pos - BOTTOMWALL) % (ROOMSIZE+1)) - (ROOMSIZE/2);
                        break;
                    case DOWN:
                        mob->x_velocity = 0;
                        mob->y_velocity = 0;
                        mob->z_velocity = -MOB_MOVEMENT_SPEED;

                        mob->x_destinationBlock = (int)mob->x_pos;
                        mob->y_destinationBlock = (int)mob->y_pos;
                        mob->z_destinationBlock = (int)mob->z_pos + (ROOMSIZE+1 - (((int)mob->z_pos - BOTTOMWALL) % (ROOMSIZE+1))) + (ROOMSIZE/2);
                        break;
                    default:
                        printf("Error\n");
                        break;
                }
                printf("Moving to: %d %d\n", mob->x_destinationBlock, mob->y_destinationBlock);
            }
            else{
                stopMob(mob);
            }
        }
        else{
            /*Currently moving, check if a wall closes in the middle of a path
              or the cell has been reached (note: no out of bounds check needed here)*/

            if(mob->x_velocity <= 0){
                /*Leftward movement - left wall check*/
                /*Is the mob at the x-position it wanted to travel to?*/
                if((int)mob->x_pos < mob->x_destinationBlock){
                    /*Destination reached*/
                    stopMob(mob);
                }
                else{
                    /*Destination ahead - check for a wall blocking the path*/

                }
            }
            else if(mob->x_velocity > 0){
                /*Rightward movement - right wall check*/
                /*Is the mob at the cell it wanted to travel to?*/
                if((int)mob->x_pos > mob->x_destinationBlock){
                    /*Destination reached*/
                    stopMob(mob);
                }
                else{
                    /*Destination ahead - check for a wall blocking the path*/
                }
            }
            else if(mob->z_velocity <= 0){
                /*Downward movement - south wall check*/
                /*Is the mob at the cell it wanted to travel to?*/
                if((int)mob->z_pos < mob->z_destinationBlock){
                    /*Destination reached*/
                    stopMob(mob);
                }
                else{
                    /*Destination ahead - check for a wall blocking the path*/
                }
            }
            else if(mob->z_velocity > 0){
                /*Upward movement - north wall check*/
                /*Is the mob at the cell it wanted to travel to?*/
                if((int)mob->z_pos > mob->z_destinationBlock){
                    /*Destination reached*/
                    stopMob(mob);
                }
                else{
                    /*Destination ahead - check for a wall blocking the path*/
                }
            }
        }
    }
    else if(mob->currentAiState == DODGING){

    }

    /*Velocity not zero - move the mob*/
    //printf("Velocity: %.2f %.2f %.2f\n", fabs(mob->x_velocity), fabs(mob->y_velocity), fabs(mob->z_velocity));
    moveMob(mob);
}

Boolean moveMob(Mob * mob){
    double newX_pos = mob->x_pos + mob->x_velocity;
    double newY_pos = mob->y_pos + mob->y_velocity;
    double newZ_pos = mob->z_pos + mob->z_velocity;

    int collidedBlock = 0;

    /*Mob is already at its destination - do nothing*/
    if(((int)newX_pos == (int)mob->x_pos) && ((int)newY_pos == (int)mob->y_pos) && ((int)newZ_pos == (int)mob->z_pos)){
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
            printf("Attempting to sidestep\n");
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
                    stopMob(mob);
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

void worldMobInit(){
    mobList = generateRandomMobs(MOB_SPAWN);
}
