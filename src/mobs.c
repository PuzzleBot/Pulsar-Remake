#include "graphics.h"

extern HighGrid waypointGrid[(GRIDSIZE * 2)][(GRIDSIZE * 2)];
Mob * mobList;

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
    static int mobHeight = FLOORHEIGHT + 2;
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
    }

    return(newMobList);
}

Boolean checkMobCollision(double destinationX, double destinationY, double destinationZ, Mob * mob){
    int i, j, k;
    Boolean collision = FALSE;

    for(i = -1; i <= 1; i++){
        for(j = -1; j <= 1; j++){
            for(k = -1; k <= 1; k++){
                if(((int)abs(destinationX + i - mob->x_pos) > 1) || ((int)abs(destinationY + j - mob->y_pos) > 1) || ((int)abs(destinationZ + k - mob->z_pos) > 1)){
                    if(world[(int)destinationX + i][(int)destinationY + j][(int)destinationZ + k] != 0){
                        collision = TRUE;
                    }
                }
            }
        }
    }

    return collision;
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
        animateSingleMob(currentMob);
        currentMob = currentMob->next;
    }
}

void processMobAI(Mob * mob){
    /*Detect line of sight, change ai state accordingly*/

    if(mob->currentAiState == ROAMING){
        /*If the mob is sitting still, start moving*/
        if((mob->x_velocity <= 0.01) && (mob->y_velocity <= 0.01) && (mob->z_velocity <= 0.01)){

        }


    }
    else if(mob->currentAiState == DODGING){

    }
}


void worldMobInit(){
    mobList = generateRandomMobs(5);
}
