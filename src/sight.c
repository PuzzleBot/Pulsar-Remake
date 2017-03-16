#include "graphics.h"

extern int allocationCounter;

BlockList * getAllBlocksOnLine(BlockList * listOfBlocks, double startX, double startY, double startZ, double endX, double endY, double endZ){
    /*Step over the line by half-units*/
    double unitLength = sqrt(pow(endX - startX, 2) + pow(endY - startY, 2) + pow(endZ - startZ, 2));
    int numberOfMovements = (int)ceil(unitLength * 2);
    double xMovement = ((endX - startX) / unitLength) / 2;
    double yMovement = ((endY - startY) / unitLength) / 2;
    double zMovement = ((endZ - startZ) / unitLength) / 2;
    int lastX = -999;
    int lastY = -999;
    int lastZ = -999;

    double currentX = startX;
    double currentY = startY;
    double currentZ = startZ;

    int i;
    for(i = 0; i < numberOfMovements; i++){
        if((world[(int)currentX][(int)currentY][(int)currentZ] != CUBE_EMPTY)
            && (((int)currentX != lastX) || ((int)currentY != lastY) || ((int)currentZ != lastZ))){
            listOfBlocks = addToBlockList(listOfBlocks, (int)currentX, (int)currentY, (int)currentZ);
            lastX = (int)currentX;
            lastY = (int)currentY;
            lastZ = (int)currentZ;
        }

        currentX = currentX + xMovement;
        currentY = currentY + yMovement;
        currentZ = currentZ + zMovement;
    }
    return(listOfBlocks);
}

BlockList * addToBlockList(BlockList * list, int newBlockX, int newBlockY, int newBlockZ){
    BlockList * newBlock = malloc(sizeof(BlockList));
    if(newBlock != NULL){
        newBlock->x = newBlockX;
        newBlock->y = newBlockY;
        newBlock->z = newBlockZ;

        newBlock->next = list;
        return newBlock;
    }
    else{
        printf("Warning: Not enough memory to calculate line of sight!\n");
        return list;
    }
}

void deleteBlockList(BlockList * list){
    BlockList * currentBlock = list;

    while(currentBlock != NULL){
        list = list->next;
        free(currentBlock);
        currentBlock = list;
    }
}

Boolean detectWallInPath(BlockList * blocksInPath){
    BlockList * currentBlock = blocksInPath;

    while(currentBlock != NULL){
        if((world[currentBlock->x][currentBlock->y][currentBlock->z] == 2) || (world[currentBlock->x][currentBlock->y][currentBlock->z] == 4) || (world[currentBlock->x][currentBlock->y][currentBlock->z] == 5) || (world[currentBlock->x][currentBlock->y][currentBlock->z] == 6)){
            return TRUE;
        }

        currentBlock = currentBlock->next;
    }

    return FALSE;
}


Boolean lineIsClear(double startX, double startY, double startZ, double endX, double endY, double endZ){
    BlockList * blocksInPath = NULL;
    Boolean lineClear;

    blocksInPath = getAllBlocksOnLine(blocksInPath, startX+0.001, startY+0.001, startZ+0.001, endX+0.001 , endY+0.001, endZ+0.001);

    /*Weird math basically converts true to false and false to true*/
    lineClear = (detectWallInPath(blocksInPath) + 1) % 2;

    deleteBlockList(blocksInPath);

    return lineClear;
}
