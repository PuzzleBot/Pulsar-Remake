#include "graphics.h"

/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void setViewOrientation(float, float, float);
extern void setOldViewPosition(float, float, float);
extern void getViewOrientation(float *, float *, float *);

/* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);



Bullet bulletArray[BULLET_ARRAY_SIZE];
int bulletCount = 0;
int nextBullet = 0;

/*Create a new bullet from the player*/
void fireBulletFromPlayer(){
    float vpX, vpY, vpZ;
    float vpOrientX, vpOrientY, vpOrientZ;

    double bulletDirectionX, bulletDirectionY, bulletDirectionZ;
    double bulletDirectionLength;
    static double playerBulletDisplacement = 1;
    static double bulletVelocity = 0.2;

    /*Bullet limit*/
    if(bulletCount < MAX_BULLETS){
        /*Limited bullet management*/
        while(bulletArray[nextBullet].existsInWorld != 0){
            nextBullet = (nextBullet + 1) % MAX_BULLETS;
        }

        bulletArray[nextBullet].existsInWorld = 1;
        bulletArray[nextBullet].lifeTimer = BULLETLIFE;

        /*Position and velocity init in here*/
        getViewPosition(&vpX, &vpY, &vpZ);
        getViewOrientation(&vpOrientX, &vpOrientY, &vpOrientZ);

        vpX = -vpX;
        vpY = -vpY;
        vpZ = -vpZ;

        vpOrientX = -vpOrientX;
        vpOrientY = -vpOrientY;
        vpOrientZ = -vpOrientZ;

        vpOrientX = fmod(vpOrientX, 360.0) * (M_PI / 180.0);
        vpOrientY = fmod(vpOrientY, 360.0) * (M_PI / 180.0);
        vpOrientZ = fmod(vpOrientZ, 360.0) * (M_PI / 180.0);

        bulletDirectionX = -sin(vpOrientY);
        bulletDirectionY = tan(vpOrientX);
        bulletDirectionZ = -cos(vpOrientY);

        //printf("Firing bullet: %.2f, %.2f, %.2f\n", bulletDirectionX, bulletDirectionY, bulletDirectionZ);

        /*Normalize the direction*/
        bulletDirectionLength = sqrt(pow(bulletDirectionX, 2) + pow(bulletDirectionY, 2) + pow(bulletDirectionZ, 2));
        bulletDirectionX = bulletDirectionX / bulletDirectionLength;
        bulletDirectionY = bulletDirectionY / bulletDirectionLength;
        bulletDirectionZ = bulletDirectionZ / bulletDirectionLength;

        //printf("Firing bullet: %.2f, %.2f, %.2f\n", bulletDirectionX, bulletDirectionY, bulletDirectionZ);


        bulletArray[nextBullet].x_pos = vpX + (bulletDirectionX * playerBulletDisplacement);
        bulletArray[nextBullet].y_pos = vpY + (bulletDirectionY * playerBulletDisplacement);
        bulletArray[nextBullet].z_pos = vpZ + (bulletDirectionZ * playerBulletDisplacement);

        bulletArray[nextBullet].x_velocity = (bulletDirectionX * bulletVelocity);
        bulletArray[nextBullet].y_velocity = (bulletDirectionY * bulletVelocity);
        bulletArray[nextBullet].z_velocity = (bulletDirectionZ * bulletVelocity);

        //printf("Firing bullet: %.2f, %.2f, %.2f\n", bulletArray[nextBullet].x_velocity, bulletArray[nextBullet].y_velocity, bulletArray[nextBullet].z_velocity);
        createMob(nextBullet, (float)bulletArray[nextBullet].x_pos, (float)bulletArray[nextBullet].y_pos, (float)bulletArray[nextBullet].z_pos, vpOrientY);


        bulletCount++;
    }
}

void fireBulletFromMobToPlayer(Mob * mob, double vpX, double vpY, double vpZ){
    double distance;
    static double bulletVelocity = 0.2;
    float vpOrientY;

    /*Limit one bullet per mob*/
    if(mob->mobBullet->existsInWorld == 0){
        distance = sqrt(pow(vpX - mob->x_pos, 2) + pow(vpY - mob->y_pos, 2) + pow(vpZ - mob->z_pos, 2));
        mob->mobBullet->x_velocity = ((vpX - mob->x_pos) / distance) * bulletVelocity;
        mob->mobBullet->y_velocity = ((vpY - mob->y_pos) / distance) * bulletVelocity;
        mob->mobBullet->z_velocity = ((vpZ - mob->z_pos) / distance) * bulletVelocity;

        mob->mobBullet->x_pos = mob->x_pos + (((vpX - mob->x_pos) / distance) * 2);
        mob->mobBullet->y_pos = mob->y_pos + (((vpY - mob->y_pos) / distance) * 2);
        mob->mobBullet->z_pos = mob->z_pos + (((vpZ - mob->z_pos) / distance) * 2);

        mob->mobBullet->existsInWorld = 1;
        mob->mobBullet->lifeTimer = BULLETLIFE;

        vpOrientY = -vpOrientY;
        vpOrientY = fmod(vpOrientY, 360.0) * (M_PI / 180.0);

        createMob(mob->bulletArrayId, (float)mob->mobBullet->x_pos, (float)mob->mobBullet->y_pos, (float)mob->mobBullet->z_pos, vpOrientY);
    }
}


void moveAllBullets(){
    int i;

    for(i = 0; i < BULLET_ARRAY_SIZE; i++){
        /*If a bullet currently exists in the world, move it*/
        if(bulletArray[i].existsInWorld == 1){
            bulletArray[i].x_pos = bulletArray[i].x_pos + bulletArray[i].x_velocity;
            bulletArray[i].y_pos = bulletArray[i].y_pos + bulletArray[i].y_velocity;
            bulletArray[i].z_pos = bulletArray[i].z_pos + bulletArray[i].z_velocity;

            /*Collision check and redraw*/
            bulletCollision(i);
        }
    }
}


void bulletCollision(int bulletId){
    int blockX, blockY, blockZ;

    blockX = floor(bulletArray[bulletId].x_pos + 0.5);
    blockY = floor(bulletArray[bulletId].y_pos + 0.5);
    blockZ = floor(bulletArray[bulletId].z_pos + 0.5);

    if((world[blockX][blockY][blockZ] != 0) || (blockX >= WORLDX) || (blockX <= 0) || (blockY >= WORLDY) || (blockY <= 0) || (blockZ >= WORLDZ) || (blockZ <= 0)){
        /*Collision happened - remove the bullet, if it hits a wall, break it*/
        removeBullet(bulletId);

        if(world[blockX][blockY][blockZ] == 6){
            world[blockX][blockY][blockZ] = 0;

            if(world[blockX][blockY+1][blockZ] == 6){
                world[blockX][blockY+1][blockZ] = 0;
            }

            if(world[blockX][blockY-1][blockZ] == 6){
                world[blockX][blockY-1][blockZ] = 0;
            }

            if(world[blockX+1][blockY][blockZ] == 6){
                world[blockX+1][blockY][blockZ] = 0;
            }

            if(world[blockX-1][blockY][blockZ] == 6){
                world[blockX-1][blockY][blockZ] = 0;
            }

            if(world[blockX][blockY][blockZ+1] == 6){
                world[blockX][blockY][blockZ+1] = 0;
            }

            if(world[blockX][blockY][blockZ-1] == 6){
                world[blockX][blockY][blockZ-1] = 0;
            }
        }
    }
    else{
        setMobPosition(bulletId, bulletArray[bulletId].x_pos, bulletArray[bulletId].y_pos, bulletArray[bulletId].z_pos, bulletArray[bulletId].yOrientation);
    }
}

void removeBullet(int bulletId){
    hideMob(bulletId);
    bulletArray[bulletId].existsInWorld = 0;
    nextBullet = bulletId;

    bulletCount--;
}

void initializeBulletArray(){
    int i;
    for(i = 0; i < BULLET_ARRAY_SIZE; i++){
        bulletArray[i].x_pos = 0;
        bulletArray[i].y_pos = 0;
        bulletArray[i].z_pos = 0;

        bulletArray[i].yOrientation = 0;

        bulletArray[i].x_velocity = 0;
        bulletArray[i].y_velocity = 0;
        bulletArray[i].z_velocity = 0;

        bulletArray[i].existsInWorld = 0;
        bulletArray[i].lifeTimer = 1500;
    }
}
