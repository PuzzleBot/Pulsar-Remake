
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

#include "graphics.h"

/* mouse function called by GLUT when a button is pressed or released */
void mouse(int, int, int, int);

/* initialize graphics library */
extern void graphicsInit(int *, char **);

/* lighting control */
extern void setLightPosition(GLfloat, GLfloat, GLfloat);
extern GLfloat* getLightPosition();

/* viewpoint control */
extern void setViewPosition(float, float, float);
extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void setViewOrientation(float, float, float);
extern void setOldViewPosition(float, float, float);
extern void getViewOrientation(float *, float *, float *);

extern void motion(int, int);

/* add cube to display list so it will be drawn */
extern int addDisplayList(int, int, int);

/* mob controls */
extern void createMob(int, float, float, float, float);
extern void setMobPosition(int, float, float, float, float);
extern void hideMob(int);
extern void showMob(int);

/* player controls */
extern void createPlayer(int, float, float, float, float);
extern void setPlayerPosition(int, float, float, float, float);
extern void hidePlayer(int);
extern void showPlayer(int);

/* 2D drawing functions */
extern void  draw2Dline(int, int, int, int, int);
extern void  draw2Dbox(int, int, int, int);
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);


/* flag which is set to 1 when flying behaviour is desired */
extern int flycontrol;
/* flag used to indicate that the test world should be used */
extern int testWorld;
/* flag to print out frames per second */
extern int fps;
/* flag to indicate the space bar has been pressed */
extern int space;
/* flag indicates the program is a client when set = 1 */
extern int netClient;
/* flag indicates the program is a server when set = 1 */
extern int netServer;
/* size of the window in pixels */
extern int screenWidth, screenHeight;
/* flag indicates if map is to be printed */
extern int displayMap;

/* frustum corner coordinates, used for visibility determination  */
extern float corners[4][3];

/* determine which cubes are visible e.g. in view frustum */
extern void ExtractFrustum();
extern void tree(float, float, float, float, float, float, int);

/********* end of extern variable declarations **************/

int allocationCounter = 0;


/* Time since the last world update, used as a clock for capping FPS */
struct timeval previousUpdate;


/*Arrays of walls - one for walls parallel to the x-axis, and one for walls parallel to the z-axis*/
extern Wall z_walls[GRIDSIZE][GRIDSIZE-1];
extern Wall x_walls[GRIDSIZE-1][GRIDSIZE];
int wallTimer = 0;
int wallAnimationTimer = 0;
int wallCount = STARTING_WALLS;
WallAnimationList * animationQueue = NULL;


/*Array of empty cells, walls between them, and pillars - basically a higher level grid of the play area*/
extern HighGrid waypointGrid[(GRIDSIZE * 2) + 1][(GRIDSIZE * 2) + 1];


/*Linked list of mobs - we're iterating over all of the live ones anyway to update, so why not a list?*/
extern Mob * mobList;
int mobTimer = 0;

extern Meteor * meteorList;
int meteorAnimationTimer = 0;

/*Player state variables*/
Boolean playerHasKey = FALSE;
Boolean stageCleared = FALSE;
int playerInvincibilityTimer = 0;
PlayerState playerState = WALKING;
Parabola playerLaunchTrajectory;

int keyBlock[3] = {0};


Boolean trace = FALSE;

/*** collisionResponse() ***/
/* -performs collision detection and response */
/*  sets new xyz  to position of the viewpoint after collision */
/* -can also be used to implement gravity by updating y position of vp*/
/* note that the world coordinates returned from getViewPosition()
 will be the negative value of the array indices */
void collisionResponse() {

    /* your collision code goes here */
    float newX, newY, newZ;
    float oldX, oldY, oldZ;

    int oldBlockX, oldBlockY, oldBlockZ;
    int newBlockX, newBlockY, newBlockZ;

    getOldViewPosition(&oldX, &oldY, &oldZ);
    getViewPosition(&newX, &newY, &newZ);

    oldX = -oldX;
    oldY = -oldY;
    oldZ = -oldZ;

    newX = -newX;
    newY = -newY;
    newZ = -newZ;

    /*Calculate the corresponding world block of the old viewpoint location*/
    oldBlockX = (int)(floor(oldX));
    oldBlockY = (int)(floor(oldY));
    oldBlockZ = (int)(floor(oldZ));

    /*Calculate the corresponding world block of the new viewpoint location*/
    newBlockX = (int)(floor(newX));
    newBlockY = (int)(floor(newY));
    newBlockZ = (int)(floor(newZ));

    /*
    printf("Old position: %.2f %.2f %.2f\n", oldX, oldY, oldZ);
    printf("New position: %.2f %.2f %.2f\n", newX, newY, oldZ);
    printf("Block: %d %d %d\n", newBlockX, newBlockY, newBlockZ);
     */

    /*Check for collision or out of bounds*/
    if((world[newBlockX][newBlockY][newBlockZ] != 0) || (world[newBlockX][newBlockY+1][newBlockZ] != 0) || (newBlockX >= WORLDX) || (newBlockX <= 0) || (newBlockY >= WORLDY) || (newBlockY <= 0) || (newBlockZ >= WORLDZ) || (newBlockZ <= 0)){
        /*Door check - if the door is collided with and the player is holding a key, there is
          no need to do the collision check*/
        if((playerHasKey == TRUE) && ((((world[newBlockX][newBlockY][newBlockZ+1] == CUBE_BLACK) && (world[newBlockX][newBlockY][newBlockZ-1] == CUBE_BLACK))
                                        || ((world[newBlockX+1][newBlockY][newBlockZ] == CUBE_BLACK) && (world[newBlockX-1][newBlockY][newBlockZ] == CUBE_BLACK)))
                                        && ((world[newBlockX][newBlockY+1][newBlockZ] == CUBE_BLACK) || (world[newBlockX][newBlockY+1][newBlockZ] == CUBE_WHITE)))){
            stageCleared = TRUE;
            return;
        }

        if(newBlockZ < oldBlockZ){
            /*Climb 1-block high walls*/
            if((world[newBlockX][newBlockY+1][newBlockZ] == 0) && (world[newBlockX][newBlockY+2][newBlockZ] == 0) && (world[newBlockX][newBlockY][newBlockZ] != 0)){
                handleSingleBlock(&newBlockX, &newY, &newBlockZ);
            }
            else{
                /*North side detection*/
                oldZ = ceil(newZ) + COLLISION_REPULSION;
                newZ = oldZ;
            }
        }
        else if(newBlockZ > oldBlockZ){
            /*Climb 1-block high walls*/
            if((world[newBlockX][newBlockY+1][newBlockZ] == 0) && (world[newBlockX][newBlockY+2][newBlockZ] == 0) && (world[newBlockX][newBlockY][newBlockZ] != 0)){
                handleSingleBlock(&newBlockX, &newY, &newBlockZ);
            }
            else{
                /*South side detection*/
                oldZ = floor(newZ) - COLLISION_REPULSION;
                newZ = oldZ;
            }
        }

        if(newBlockX < oldBlockX){
            /*Climb 1-block high walls*/
            if((world[newBlockX][newBlockY+1][newBlockZ] == 0) && (world[newBlockX][newBlockY+2][newBlockZ] == 0) && (world[newBlockX][newBlockY][newBlockZ] != 0)){
                handleSingleBlock(&newBlockX, &newY, &newBlockZ);
            }
            else{
                /*Right side detection*/
                oldX = ceil(newX) + COLLISION_REPULSION;
                newX = oldX;
            }
        }
        else if(newBlockX > oldBlockX){
            /*Climb 1-block high walls*/
            if((world[newBlockX][newBlockY+1][newBlockZ] == 0) && (world[newBlockX][newBlockY+2][newBlockZ] == 0) && (world[newBlockX][newBlockY][newBlockZ] != 0)){
                handleSingleBlock(&newBlockX, &newY, &newBlockZ);
            }
            else{
                /*Left side detection*/
                oldX = floor(newX) - COLLISION_REPULSION;
                newX = oldX;
            }
        }

        if(newBlockY > oldBlockY){
            /*Bottom side detection*/
            oldY = floor(newY) - COLLISION_REPULSION;
            newY = oldY;
        }

        /*Different collision detection for the floor - see below*/
        if(!(newBlockY < oldBlockY)){
            setViewPosition(-newX, -newY, -newZ);
        }
    }

    if((newY < oldY) &&
       ((world[newBlockX][(int)floor(newY - UPWARD_REPULSION)][newBlockZ] != 0) || ((int)floor(newY - UPWARD_REPULSION) < 0))){
        /*Top side detection - different in order to keep the
         viewpoint at a certain height*/
        oldY = ceil(newY - UPWARD_REPULSION) + UPWARD_REPULSION - 0.01;
        newY = oldY;

        setViewPosition(-newX, -newY, -newZ);
    }
}


/******* draw2D() *******/
/* draws 2D shapes on screen */
/* use the following functions: 			*/
/*	draw2Dline(int, int, int, int, int);		*/
/*	draw2Dbox(int, int, int, int);			*/
/*	draw2Dtriangle(int, int, int, int, int, int);	*/
/*	set2Dcolour(float []); 				*/
/* colour must be set before other functions are called	*/
void draw2D() {

    if (testWorld) {
        /* draw some sample 2d shapes */
        GLfloat green[] = {0.0, 0.5, 0.0, 0.5};
        set2Dcolour(green);
        draw2Dline(0, 0, 500, 500, 15);
        draw2Dtriangle(0, 0, 200, 200, 0, 200);

        GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
        set2Dcolour(black);
        draw2Dbox(500, 380, 524, 388);
    } else {

        /* your code goes here */

        /*Draw the minimap*/
        if(displayMap == 1){
            drawSmallMinimap();
        }
        else if(displayMap == 2){
            drawFullMap();
        }

        drawGameplayUI();

    }

}


/*** update() ***/
/* background process, it is called when there are no other events */
/* -used to control animations and perform calculations while the  */
/*  system is running */
/* -gravity must also implemented here, duplicate collisionResponse */
void update() {
    int i, j, k;
    struct timeval currentTime;
    int secondsSinceUpdate;
    int microSecsSinceUpdate;
    float *la;


    /* sample animation for the test world, don't remove this code */
    /* -demo of animating mobs */
    if (testWorld) {

        /* sample of rotation and positioning of mob */
        /* coordinates for mob 0 */
        static float mob0x = 50.0, mob0y = 25.0, mob0z = 52.0;
        static float mob0ry = 0.0;
        static int increasingmob0 = 1;
        /* coordinates for mob 1 */
        static float mob1x = 50.0, mob1y = 25.0, mob1z = 52.0;
        static float mob1ry = 0.0;
        static int increasingmob1 = 1;

        /* move mob 0 and rotate */
        /* set mob 0 position */
        setMobPosition(0, mob0x, mob0y, mob0z, mob0ry);

        /* move mob 0 in the x axis */
        if (increasingmob0 == 1)
            mob0x += 0.2;
        else
            mob0x -= 0.2;
        if (mob0x > 50) increasingmob0 = 0;
        if (mob0x < 30) increasingmob0 = 1;

        /* rotate mob 0 around the y axis */
        mob0ry += 1.0;
        if (mob0ry > 360.0) mob0ry -= 360.0;

        /* move mob 1 and rotate */
        setMobPosition(1, mob1x, mob1y, mob1z, mob1ry);

        /* move mob 1 in the z axis */
        /* when mob is moving away it is visible, when moving back it */
        /* is hidden */
        if (increasingmob1 == 1) {
            mob1z += 0.2;
            showMob(1);
        } else {
            mob1z -= 0.2;
            hideMob(1);
        }
        if (mob1z > 72) increasingmob1 = 0;
        if (mob1z < 52) increasingmob1 = 1;

        /* rotate mob 1 around the y axis */
        mob1ry += 1.0;
        if (mob1ry > 360.0) mob1ry -= 360.0;
        /* end testworld animation */

    } else {
        /* your code goes here */
        int l;

        /*Performance cap - 30 FPS*/
        gettimeofday(&currentTime, NULL);

        /*Get the number of seconds since the last update. Only allow
          updates 30 times per second. (1 000 000 microseconds/sec divided by 30)*/
        secondsSinceUpdate = difftime(currentTime.tv_sec, previousUpdate.tv_sec);
        microSecsSinceUpdate = (int)currentTime.tv_usec - (int)previousUpdate.tv_usec;

        if((microSecsSinceUpdate > 1000000/FPS_CAP) || (secondsSinceUpdate >= 1)){
            /*Gravity*/
            float viewX;
            float viewY;
            float viewZ;

            if(stageCleared == TRUE){
                mazeDoor();
                stageCleared = FALSE;
                return;
            }

            if(playerHasKey == FALSE){
                world[keyBlock[0]][keyBlock[1]][keyBlock[2]] = CUBE_WHITE;
            }
            else{
                world[keyBlock[0]][keyBlock[1]][keyBlock[2]] = CUBE_EMPTY;
            }


            if((flycontrol == 0) && (playerState != FLYING)){
                getViewPosition(&viewX, &viewY, &viewZ);
                setOldViewPosition(viewX, viewY, viewZ);
                viewY = viewY + GRAVITY;
                setViewPosition(viewX, viewY, viewZ);
                collisionResponse();
            }

            if(playerState == FLYING){
                iterateBounceMovement();
            }

            /*Wall Movement*/
            wallTimer++;
            if(wallTimer >= FPS_CAP){
                wallTimer = 0;

                /*Open/close walls at random*/
                for(l = 0; l < WALL_TOGGLE_COUNT; l++){
                    toggleRandomWall();
                }
                //toggleTwoWalls();*/
                //printWaypointGrid();
            }

            /*AI movement*/
            mobTimer++;
            if(mobTimer >= FPS_CAP/15){
                mobTimer = 0;
                animateAllMobs(mobList);
            }

            /*Wall animation*/
            wallAnimationTimer++;
            if(wallAnimationTimer >= FPS_CAP/10){
                wallAnimationTimer = 0;
                processAllWallAnimations();
            }

            meteorAnimationTimer++;
            if(meteorAnimationTimer >= FPS_CAP/15){
                meteorAnimationTimer = 0;
                meteorList = animateAllMeteors(meteorList);
            }

            /*Bullet movement*/
            moveAllBullets();

            if(playerInvincibilityTimer > 0){
                playerInvincibilityTimer--;
            }

            glutPostRedisplay();
            gettimeofday(&previousUpdate, NULL);
        }

    }
}


/* called by GLUT when a mouse button is pressed or released */
/* -button indicates which button was pressed or released */
/* -state indicates a button down or button up event */
/* -x,y are the screen coordinates when the mouse is pressed or */
/*  released */
void mouse(int button, int state, int x, int y) {

    if (button == GLUT_LEFT_BUTTON){
        //printf("left button - ");
        if(state != GLUT_UP){
            fireBulletFromPlayer();
        }
    }
    else if (button == GLUT_MIDDLE_BUTTON){
        //printf("middle button - ");
    }
    else{
        //printf("right button - ");
    }

    if (state == GLUT_UP){
        //printf("up - ");
    }
    else{
        //printf("down - ");
    }

    //printf("%d %d\n", x, y);
}



int main(int argc, char** argv)
{
    int i, j, k;

    /* initialize the graphics system */
    graphicsInit(&argc, argv);

    /* the first part of this if statement builds a sample */
    /* world which will be used for testing */
    /* DO NOT remove this code. */
    /* Put your code in the else statment below */
    /* The testworld is only guaranteed to work with a world of
     with dimensions of 100,50,100. */
    if (testWorld == 1) {
        /* initialize world to empty */
        for(i=0; i<WORLDX; i++)
            for(j=0; j<WORLDY; j++)
                for(k=0; k<WORLDZ; k++)
                    world[i][j][k] = 0;

        /* some sample objects */
        /* build a red platform */
        for(i=0; i<WORLDX; i++) {
            for(j=0; j<WORLDZ; j++) {
                world[i][24][j] = 3;
            }
        }
        /* create some green and blue cubes */
        world[50][25][50] = 1;
        world[49][25][50] = 1;
        world[49][26][50] = 1;
        world[52][25][52] = 2;
        world[52][26][52] = 2;

        /* blue box shows xy bounds of the world */
        for(i=0; i<WORLDX-1; i++) {
            world[i][25][0] = 2;
            world[i][25][WORLDZ-1] = 2;
        }
        for(i=0; i<WORLDZ-1; i++) {
            world[0][25][i] = 2;
            world[WORLDX-1][25][i] = 2;
        }

        /* create two sample mobs */
        /* these are animated in the update() function */
        createMob(0, 50.0, 25.0, 52.0, 0.0);
        createMob(1, 50.0, 25.0, 52.0, 0.0);

        /* create sample player */
        createPlayer(0, 52.0, 27.0, 52.0, 0.0);

    } else {

        /* your code to build the world goes here */
        /*RNG seed*/
        srand(time(NULL));
        resetMaze();

        glutWarpPointer(512, 384);
        motion(512, 384);

        if(CURSOR_MODE == 1){
            glutSetCursor(GLUT_CURSOR_NONE);
        }


        /*Debug line for viewing the world from the top, uncomment to use*/
        /*setViewPosition(-10, -38, -30);*/

        gettimeofday(&previousUpdate, NULL);

    }

    /* starts the graphics processing loop */
    /* code after this will not run until the program exits */
    glutMainLoop();
    return 0;
}
