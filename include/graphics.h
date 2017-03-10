
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

        /* world size and storage array */
#define WORLDX 100
#define WORLDY 50
#define WORLDZ 100
GLubyte  world[WORLDX][WORLDY][WORLDZ];

/*Maximum FPS*/
#define FPS_CAP 60


/*2D UI constants for the minimap*/
#define UI_MAP_BORDER_WIDTH 3

/*Map border calculation generalization:
  bottom = top of the screen - (size of the world in map blocks + downward screen shift)
  top = top of the screen - downward screen shift
  -- same idea for the left and right parts of the border*/
#define UI_MAP_BOTTOM (screenHeight - ((TOPWALL - BOTTOMWALL + UI_MAP_DISPLACEMENT)*UI_SQUARE_DIM_VERT))
#define UI_MAP_LEFT (screenWidth - ((RIGHTWALL - LEFTWALL + UI_MAP_DISPLACEMENT)*UI_SQUARE_DIM_HORIZ))
#define UI_MAP_TOP (screenHeight - ((UI_MAP_DISPLACEMENT - 1) * UI_SQUARE_DIM_VERT))
#define UI_MAP_RIGHT (screenWidth - ((UI_MAP_DISPLACEMENT - 1) * UI_SQUARE_DIM_HORIZ))

#define UI_SQUARE_DIM_HORIZ ((int)screenWidth/200) // Default: 5x5
#define UI_SQUARE_DIM_VERT ((int)screenHeight/141)
#define UI_MAP_DISPLACEMENT 2

#define UI_PLAYER_DIM_HORIZ ((int)screenWidth/511) // Default: 2x2
#define UI_PLAYER_DIM_VERT ((int)screenHeight/352)


/*2D UI constants for the full map*/
#define UI_FULLMAP_BORDER_WIDTH 5

/*Map border calculation generalization:
 bottom = center of the screen - half the size of the world in map blocks
 top = center of the screen + half the size of the world in map blocks
 -- same idea for the left and right parts of the border*/
#define UI_FULLMAP_BOTTOM ((screenHeight/2) - (((TOPWALL-BOTTOMWALL)*UI_FULLSQUARE_DIM_VERT)/2))
#define UI_FULLMAP_LEFT ((screenWidth/2) - (((RIGHTWALL-LEFTWALL)*UI_FULLSQUARE_DIM_HORIZ)/2))
#define UI_FULLMAP_TOP ((screenHeight/2) + (((TOPWALL-BOTTOMWALL)*UI_FULLSQUARE_DIM_VERT)/2))
#define UI_FULLMAP_RIGHT ((screenWidth/2) + (((RIGHTWALL-LEFTWALL)*UI_FULLSQUARE_DIM_HORIZ)/2))

#define UI_FULLSQUARE_DIM_HORIZ ((int)screenWidth/100) // Default: 10x10
#define UI_FULLSQUARE_DIM_VERT ((int)screenHeight/70)

#define UI_FULLPLAYER_DIM_HORIZ ((int)screenWidth/255) // Default: 4x4
#define UI_FULLPLAYER_DIM_VERT ((int)screenHeight/176)


#define UI_BULLET_DIM_HORIZ ((int)screenWidth/511)
#define UI_BULLET_DIM_VERT ((int)screenHeight/352)



/*World building constants*/
#define MAX_DISPLAY_LIST 500000

#define RIGHTWALL (LEFTWALL + (ROOMSIZE * GRIDSIZE) + GRIDSIZE)
#define TOPWALL (BOTTOMWALL + (ROOMSIZE * GRIDSIZE) + GRIDSIZE)

#define LEFTWALL 20
#define BOTTOMWALL 20
#define ROOMSIZE 9
#define GRIDSIZE 6
#define FLOORHEIGHT 10

#define STARTING_WALLS 25
#define WALL_HEIGHT 5

/*Collision and gravity*/
#define COLLISION_REPULSION 0.15
#define UPWARD_REPULSION 0.7
#define GRAVITY 0.16

#define FLOAT_VARIANCE 0.001

/*Limit the number of player bullets on the screen at once*/
#define MAX_BULLETS 12
#define BULLET_ARRAY_SIZE MAX_BULLETS + MOB_SPAWN
#define BULLETLIFE 1500

#define MOB_BULLET_ARRAY_START MAX_BULLETS
#define MOB_MOVEMENT_SPEED 0.80
#define MOB_SPAWN 1


typedef enum{FALSE, TRUE} Boolean;


/*Wall state data type*/
typedef enum{OPEN, OPENING, CLOSED, CLOSING} WallState;

/*Wall data structure containing the state of each wall (opening, closing, etc), and may contain
  other data in the future*/
typedef struct{
    WallState state;
} Wall;

typedef struct anim_list{
    /*Location of the wall in the wall array*/
    int targetWallIndex[2];

    /*Which array is it in - the x_walls array (1) or z_walls array (0)?*/
    int isXwall;

    /*Current frame of animation - animation is completed when this reaches zero*/
    int animationState;

    /*Linked list*/
    struct anim_list * next;
    struct anim_list * prev;
} AnimationList;


/*Data structure representing a bullet*/
typedef struct{
    double x_pos;
    double y_pos;
    double z_pos;

    double yOrientation;

    double x_velocity;
    double y_velocity;
    double z_velocity;

    int existsInWorld;
    int lifeTimer;
} Bullet;


/*Mob structures*/
typedef enum{ROAMING, DODGING} MobState;
typedef enum{ORBITER, PULSAR} MobType;

typedef struct MobChain{
    MobType type;
    double x_velocity;
    double y_velocity;
    double z_velocity;

    double x_pos;
    double y_pos;
    double z_pos;

    int x_destinationBlock;
    int y_destinationBlock;
    int z_destinationBlock;

    int currentAnimationFrame;
    MobState currentAiState;
    int currentHighGridCell[2];

    Bullet * mobBullet;
    int bulletArrayId;

    struct MobChain * next;
    struct MobChain * prev;
} Mob;

typedef struct BlockChain{
    int x;
    int y;
    int z;

    struct BlockChain * next;
} BlockList;


/*Higher level grid - for managing pathfinding and waypoints*/
typedef enum{HIGHGRID_WALL, HIGHGRID_PILLAR, HIGHGRID_CELL} HighGridType;
typedef enum{LEFT, RIGHT, UP, DOWN} Direction;

typedef struct{
    HighGridType type;
    Wall * correspondingWall;
} HighGrid;


/* Wall functions */

/*Builds all non-moving world parts, including the floor, pillars, and border*/
void buildStaticObjects();

/*Initializes the maze with movable walls, amount of walls dependent on the
  constant STARTING_WALLS*/
void initializeWalls();

/*Chooses a wall at random, then queues an animation for removal if it is there,
  or an animation for building it if it is not there*/
void toggleRandomWall();

/*Save as the above except it chooses both one existing wall and one non-existing
  wall at random when used*/
void toggleTwoWalls();

/*Queues an animation to be done on the next processing cycle*/
void addToAnimationQueue(WallState animationType, int isXwall, int row, int col);

/*Perform one frame of each queued animation, removes the queued animation
  once it completes using the function below*/
void processAllAnimations();

/*Removes an animation from the animation queue*/
void deleteFromAnimationQueue(AnimationList * toDelete);

void printXWalls();
void printZWalls();



/*Draws a minimap in the top-right corner*/
void drawSmallMinimap();

/*Draws a large map in the center of the screen*/
void drawFullMap();

/*Fires a bullet from the player based on their view*/
void fireBulletFromPlayer();

void fireBulletFromMobToPlayer(Mob * mob, double vpX, double vpY, double vpZ);

void moveAllBullets();

void bulletCollision(int bulletId);

void removeBullet(int bulletId);


/*Mob Functions*/
Mob * createNewMob(MobType type, double x, double y, double z);
void generateValidMobPosition(int * x, int * y, int * z);
Mob * mobAddToFront(Mob * mobList, Mob * newMob);
Mob * mobDeleteFromFront(Mob * mobList);
Mob * generateRandomMobs(int count);
int checkMobCollision(double destinationX, double destinationY, double destinationZ, Mob * mob);
void clearMobSpace(int coreX, int coreY, int coreZ);
void animateSingleMob(Mob * mob);
void animateAllMobs();
void processMobAI(Mob * mob);
Boolean moveMob(Mob * mob);
void stopMob(Mob * mob);
void worldMobInit();

/*Waypoint grid functions*/
void getCorrespondingHighGridIndex(int * xIndex, int * yIndex, int xBlock, int zBlock);
void initWaypointGrid();
void printWaypointGrid();

/*Line of sight functions*/
BlockList * getAllBlocksOnLine(BlockList * listofBlocks, double startX, double startY, double startZ, double endX, double endY, double endZ);
BlockList * addToBlockList(BlockList * list, int newBlockX, int newBlockY, int newBlockZ);
void deleteBlockList(BlockList * list);
Boolean detectWallInPath(BlockList * blocksInPath);
