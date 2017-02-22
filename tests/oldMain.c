
/* Derived from scene.c in the The OpenGL Programming Guide */
/* Keyboard and mouse rotation taken from Swiftless Tutorials #23 Part 2 */
/* http://www.swiftless.com/tutorials/opengl/camera2.html */

/* Frames per second code taken from : */
/* http://www.lighthouse3d.com/opengl/glut/index.php?fps */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

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
extern void getViewOrientation(float *, float *, float *);

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

/* Global game clock */
time_t previousUpdate;


/*Arrays of walls - one for walls parallel to the x-axis, and one for walls parallel to the z-axis*/
Wall x_walls[GRIDSIZE][GRIDSIZE+1];
Wall z_walls[GRIDSIZE+1][GRIDSIZE];
int wallTimer = 0;
AnimationList * animationQueue = NULL;



/*** collisionResponse() ***/
/* -performs collision detection and response */
/*  sets new xyz  to position of the viewpoint after collision */
/* -can also be used to implement gravity by updating y position of vp*/
/* note that the world coordinates returned from getViewPosition()
 will be the negative value of the array indices */
void collisionResponse() {
    
    /* your collision code goes here */
    float newX;
    float newY;
    float newZ;
    
    float oldX;
    float oldY;
    float oldZ;
    
    getOldViewPosition(&oldX, &oldY, &oldZ);
    getViewPosition(&newX, &newY, &newZ);
    
    oldX = -oldX;
    oldY = -oldY;
    oldZ = -oldZ;
    
    newX = -newX;
    newY = -newY;
    newZ = -newZ;
    
    /*
     printf("Old position: %.2f %.2f %.2f\n", oldX, oldY, oldZ);
     printf("New position: %.2f %.2f %.2f\n", newX, newY, oldZ);
     printf("Block: %d %d %d\n", (int)floor(newX), (int)floor(newY), (int)floor(newZ));
     */
    
    if(world[(int)floor(newX)][(int)floor(newY)][(int)floor(newZ)] != 0){
        /*printf("Collision: %d %d %d\n", (int)floor(newX), (int)floor(newY), (int)floor(newZ));*/
        if((int)floor(newZ) < (int)floor(oldZ)){
            /*Top side detection*/
            oldZ = ceil(newZ) + COLLISION_REPULSION;
            newZ = oldZ;
        }
        else if((int)floor(newZ) > (int)floor(oldZ)){
            /*Bottom side detection*/
            oldZ = floor(newZ) - COLLISION_REPULSION;
            newZ = oldZ;
        }
        
        if((int)floor(newX) < (int)floor(oldX)){
            /*Right side detection*/
            oldX = ceil(newX) + COLLISION_REPULSION;
            newX = oldX;
        }
        else if((int)floor(newX) > (int)floor(oldX)){
            /*Left side detection*/
            oldX = floor(newX) - COLLISION_REPULSION;
            newX = oldX;
        }
        
        if((int)floor(newY) < (int)floor(oldY)){
            /*Top side detection*/
            oldY = ceil(newY) + COLLISION_REPULSION;
            newY = oldY;
        }
        else if((int)floor(newY) > (int)floor(oldY)){
            /*Bottom side detection*/
            oldY = floor(newY) - COLLISION_REPULSION;
            newY = oldY;
        }
        
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
        
    }
    
}


/*** update() ***/
/* background process, it is called when there are no other events */
/* -used to control animations and perform calculations while the  */
/*  system is running */
/* -gravity must also implemented here, duplicate collisionResponse */
void update() {
    int i, j, k;
    time_t currentTime;
    double timeSinceUpdate;
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
        
        /*Performance cap - 30 FPS*/
        currentTime = time(&currentTime);
        
        /*Get the number of seconds since the last update. Only allow
         updates 30 times per second.*/
        timeSinceUpdate = difftime(currentTime, previousUpdate);
        if(timeSinceUpdate >= 0.0333){
            /*Gravity*/
            float viewX;
            float viewY;
            float viewZ;
            
            if(flycontrol == 0){
                getViewPosition(&viewX, &viewY, &viewZ);
                viewY = viewY + 0.2;
                setViewPosition(viewX, viewY, viewZ);
                collisionResponse();
                glutPostRedisplay();
            }
            
            wallTimer++;
            if(wallTimer >= 30){
                wallTimer = 0;
                toggleRandomWall();
            }
            
            processAllAnimations();
        }
        
    }
}


/* called by GLUT when a mouse button is pressed or released */
/* -button indicates which button was pressed or released */
/* -state indicates a button down or button up event */
/* -x,y are the screen coordinates when the mouse is pressed or */
/*  released */
void mouse(int button, int state, int x, int y) {
    
    if (button == GLUT_LEFT_BUTTON)
        printf("left button - ");
    else if (button == GLUT_MIDDLE_BUTTON)
        printf("middle button - ");
    else
        printf("right button - ");
    
    if (state == GLUT_UP)
        printf("up - ");
    else
        printf("down - ");
    
    printf("%d %d\n", x, y);
}

