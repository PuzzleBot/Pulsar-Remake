#include "graphics.h"

extern void  draw2Dline(int, int, int, int, int);
extern void  draw2Dbox(int, int, int, int);
extern void  draw2Dtriangle(int, int, int, int, int, int);
extern void  set2Dcolour(float []);

extern void getViewPosition(float *, float *, float *);
extern void getOldViewPosition(float *, float *, float *);
extern void getViewOrientation(float *, float *, float *);

/* size of the window in pixels */
extern int screenWidth, screenHeight;
/* flag indicates if map is to be printed */
extern int displayMap;

extern Boolean playerHasKey;

extern Bullet bulletArray[BULLET_ARRAY_SIZE];

extern int playerInvincibilityTimer;

extern Mob * mobList;


/*For tracking the transparency of the hit indicator*/


/*Wall and floor colours*/
GLfloat blue[] = {0.0, 0.0, 1.0, 0.5};
GLfloat purple[] = {1.0, 0.0, 1.0, 0.5};
GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
GLfloat white[] = {1.0, 1.0, 1.0, 1.0};

/*Mob colours*/
GLfloat red[]   = {1.0, 0.0, 0.0, 0.5};
GLfloat green[] = {0.0, 1.0, 0.0, 0.5};
GLfloat yellow[]   = {1.0, 1.0, 0.0, 0.5};
GLfloat orange[]   = {1.0, 0.64, 0.0, 0.5};

GLfloat lightBlue[]  = {0.25, 0.85, 1.0, 0.5};

GLfloat pickupRed[]   = {1.0, 0.0, 0.0, 1.0};
GLfloat pickupGreen[] = {0.0, 1.0, 0.0, 1.0};
GLfloat pickupBlue[]  = {0.25, 0.85, 1.0, 1.0};

GLfloat borderColour[] = {0.0, 0.0, 0.0, 1.0};
GLfloat playerColour[] = {1.0, 1.0, 0.0, 1.0};
GLfloat bulletColour[] = {0.0, 1.0, 1.0, 1.0};

GLfloat orbiterColour[] = {1.0, 1.0, 0.0, 1.0};
GLfloat orbiterRingColour[] = {1.0, 0.64, 0.0, 1.0};
GLfloat pulsarColour[] = {1.0, 0.0, 0.0, 1.0};

GLfloat transparentColour[] = {0.0, 0.0, 0.0, 0.0};
GLfloat hitIndicatorColour[] = {1.0, 0.0, 0.0, 0.0};

void drawSmallMinimap(){
    float vpX, vpY, vpZ;
    int i, j, k;
    int currentBufferValue = 0;
    Mob * currentMob = mobList;

    /*Array acting like a z-buffer for the map, except it uses
     the height (y) instead.
     0 = unused map space
     1 = block on the map at floor height
     2 = block 1 above floor height
     ...etc
     */
    int ui_Ybuffer[WORLDX][WORLDZ] = {0};
    int currentSquareLeft;
    int currentSquareBottom;

    int playerSquareLeft;
    int playerSquareBottom;

    int bulletSquareLeft;
    int bulletSquareBottom;

    int mobSquareLeft;
    int mobSquareBottom;


    /*Draw the player*/
    getViewPosition(&vpX, &vpY, &vpZ);
    vpX = -vpX;
    vpY = -vpY;
    vpZ = -vpZ;

    playerSquareLeft = UI_MAP_RIGHT - (vpX - LEFTWALL)*UI_SQUARE_DIM_HORIZ;
    playerSquareBottom = UI_MAP_BOTTOM + (vpZ - BOTTOMWALL)*UI_SQUARE_DIM_VERT;

    set2Dcolour(playerColour);
    draw2Dbox(playerSquareLeft,
              playerSquareBottom,
              (playerSquareLeft) + UI_PLAYER_DIM_HORIZ,
              (playerSquareBottom) + UI_PLAYER_DIM_VERT);

    /*Draw mobs*/
    while(currentMob != NULL){
        mobSquareLeft = UI_MAP_RIGHT - ((int)currentMob->x_pos - LEFTWALL + 2)*UI_SQUARE_DIM_HORIZ;
        mobSquareBottom = UI_MAP_BOTTOM + ((int)currentMob->z_pos - BOTTOMWALL - 1)*UI_SQUARE_DIM_VERT;
        if(currentMob->type == ORBITER){
            set2Dcolour(orbiterColour);
            draw2Dbox(mobSquareLeft + (UI_SQUARE_DIM_HORIZ),
                      mobSquareBottom + (UI_SQUARE_DIM_HORIZ),
                      mobSquareLeft + (UI_SQUARE_DIM_HORIZ*2),
                      mobSquareBottom + (UI_SQUARE_DIM_VERT*2));
            set2Dcolour(black);
            draw2Dbox(mobSquareLeft + (UI_SQUARE_DIM_HORIZ*0.5),
                      mobSquareBottom + (UI_SQUARE_DIM_HORIZ*0.5),
                      mobSquareLeft + (UI_SQUARE_DIM_HORIZ*2.6),
                      mobSquareBottom + (UI_SQUARE_DIM_VERT*2.6));
            set2Dcolour(orbiterRingColour);
            draw2Dbox(mobSquareLeft,
                      mobSquareBottom,
                      mobSquareLeft + (UI_SQUARE_DIM_HORIZ*3),
                      mobSquareBottom + (UI_SQUARE_DIM_VERT*3));
        }
        else{
            set2Dcolour(pulsarColour);
            draw2Dline(mobSquareLeft,
                       mobSquareBottom,
                       mobSquareLeft + (UI_SQUARE_DIM_HORIZ*3),
                       mobSquareBottom + (UI_SQUARE_DIM_VERT*3),
                       ((UI_SQUARE_DIM_HORIZ + UI_SQUARE_DIM_VERT)) / (2 * 2));
            draw2Dline(mobSquareLeft,
                       mobSquareBottom + (UI_SQUARE_DIM_VERT*3),
                       mobSquareLeft + (UI_SQUARE_DIM_HORIZ*3),
                       mobSquareBottom,
                       ((UI_SQUARE_DIM_HORIZ + UI_SQUARE_DIM_VERT)) / (2 * 2));
            set2Dcolour(black);
            draw2Dbox(mobSquareLeft + (UI_SQUARE_DIM_HORIZ*0.9),
                      mobSquareBottom + (UI_SQUARE_DIM_HORIZ*0.9),
                      mobSquareLeft + (UI_SQUARE_DIM_HORIZ*2.1),
                      mobSquareBottom + (UI_SQUARE_DIM_VERT*2.1));
            set2Dcolour(pulsarColour);
            draw2Dbox(mobSquareLeft + (UI_SQUARE_DIM_HORIZ*0.6),
                      mobSquareBottom + (UI_SQUARE_DIM_HORIZ*0.6),
                      mobSquareLeft + (UI_SQUARE_DIM_HORIZ*2.3),
                      mobSquareBottom + (UI_SQUARE_DIM_VERT*2.3));
        }
        currentMob = currentMob->next;
    }


    /*Draw projectiles*/
    for(i = 0; i < BULLET_ARRAY_SIZE; i++){
        if(bulletArray[i].existsInWorld == 1){
            bulletSquareLeft = UI_MAP_RIGHT - (bulletArray[i].x_pos - LEFTWALL - 0)*UI_SQUARE_DIM_HORIZ;
            bulletSquareBottom = UI_MAP_BOTTOM + (bulletArray[i].z_pos - BOTTOMWALL)*UI_SQUARE_DIM_VERT;

            set2Dcolour(bulletColour);
            draw2Dbox(bulletSquareLeft,
                  bulletSquareBottom,
                  (bulletSquareLeft) + UI_BULLET_DIM_HORIZ,
                  (bulletSquareBottom) + UI_BULLET_DIM_VERT);
        }
    }


    set2Dcolour(borderColour);
    /*Draw the minimap border - left wall, bottom wall, right wall, top wall*/

    draw2Dline(UI_MAP_LEFT - (UI_MAP_BORDER_WIDTH / 2),
               UI_MAP_BOTTOM - UI_MAP_BORDER_WIDTH,
               UI_MAP_LEFT - (UI_MAP_BORDER_WIDTH / 2),
               UI_MAP_TOP + UI_MAP_BORDER_WIDTH,
               UI_MAP_BORDER_WIDTH);

    draw2Dline(UI_MAP_LEFT,
               UI_MAP_BOTTOM - (UI_MAP_BORDER_WIDTH / 2) - 1,
               UI_MAP_RIGHT,
               UI_MAP_BOTTOM - (UI_MAP_BORDER_WIDTH / 2) - 1,
               UI_MAP_BORDER_WIDTH);

    draw2Dline(UI_MAP_RIGHT + (UI_MAP_BORDER_WIDTH / 2),
               UI_MAP_BOTTOM - UI_MAP_BORDER_WIDTH,
               UI_MAP_RIGHT + (UI_MAP_BORDER_WIDTH / 2),
               UI_MAP_TOP + UI_MAP_BORDER_WIDTH,
               UI_MAP_BORDER_WIDTH);

    draw2Dline(UI_MAP_LEFT,
               UI_MAP_TOP + (UI_MAP_BORDER_WIDTH / 2),
               UI_MAP_RIGHT,
               UI_MAP_TOP + (UI_MAP_BORDER_WIDTH / 2),
               UI_MAP_BORDER_WIDTH);

    for(i = LEFTWALL; i <= RIGHTWALL; i++){
        for(j = FLOORHEIGHT+WALL_HEIGHT; j >= FLOORHEIGHT; j--){
            /*Draw the map from top to bottom to prevent overlap (y)*/

            currentBufferValue = j - FLOORHEIGHT + 1;
            for(k = BOTTOMWALL; k <= TOPWALL; k++){
                if((world[i][j][k] != 0) && (ui_Ybuffer[i][k] < currentBufferValue)){
                    /*Select a colour to match the world's cube colour*/
                    switch(world[i][j][k]){
                        case 2:
                            /*Blue*/
                            set2Dcolour(blue);
                            break;
                        case 4:
                            /*Black*/
                            set2Dcolour(black);
                            break;
                        case 5:
                            /*White*/
                            set2Dcolour(white);
                            break;
                        case 6:
                            /*Purple*/
                            set2Dcolour(purple);
                            break;
                        case 9:
                            /*Light Blue*/
                            set2Dcolour(lightBlue);
                            break;
                        case 11:
                            set2Dcolour(pickupRed);
                            break;
                        case 12:
                            set2Dcolour(pickupGreen);
                            break;
                        case 13:
                            set2Dcolour(pickupBlue);
                            break;
                        default:
                            /*Black*/
                            set2Dcolour(black);
                            break;
                    }

                    /*Draw from right to left, bottom to top*/
                    currentSquareLeft = UI_MAP_RIGHT - (i - LEFTWALL + 1)*UI_SQUARE_DIM_HORIZ;
                    currentSquareBottom = UI_MAP_BOTTOM + (k - BOTTOMWALL)*UI_SQUARE_DIM_VERT;

                    draw2Dbox((currentSquareLeft),
                              (currentSquareBottom),
                              (currentSquareLeft) + UI_SQUARE_DIM_HORIZ,
                              (currentSquareBottom) + UI_SQUARE_DIM_VERT);

                    /*Key Highlight
                    if((world[i][j][k] == 5) && (j <= FLOORHEIGHT+2)){
                        set2Dcolour(yellow);
                        draw2Dbox((currentSquareLeft - 2),
                                (currentSquareBottom - 2),
                                (currentSquareLeft) + UI_SQUARE_DIM_HORIZ + 4,
                                (currentSquareBottom) + UI_SQUARE_DIM_VERT + 2);
                        set2Dcolour(white);
                    }*/

                    ui_Ybuffer[i][k] = currentBufferValue;
                }
            }
        }
    }
}


void drawFullMap(){
    float vpX, vpY, vpZ;
    int i, j, k;
    int currentBufferValue = 0;
    Mob * currentMob = mobList;

    /*Array acting like a z-buffer for the map, except it uses
     the height (y) instead.
     0 = unused map space
     1 = block on the map at floor height
     2 = block 1 above floor height
     ...etc
     */
    int ui_Ybuffer[WORLDX][WORLDZ] = {0};
    int currentSquareLeft;
    int currentSquareBottom;

    int playerSquareLeft;
    int playerSquareBottom;

    int bulletSquareLeft;
    int bulletSquareBottom;

    int mobSquareLeft;
    int mobSquareBottom;

    /*Draw the player*/
    getViewPosition(&vpX, &vpY, &vpZ);
    vpX = -vpX;
    vpY = -vpY;
    vpZ = -vpZ;

    playerSquareLeft = UI_FULLMAP_RIGHT - (vpX - LEFTWALL - 1)*UI_FULLSQUARE_DIM_HORIZ;
    playerSquareBottom = UI_FULLMAP_BOTTOM + (vpZ - BOTTOMWALL)*UI_FULLSQUARE_DIM_VERT;

    set2Dcolour(playerColour);
    draw2Dbox(playerSquareLeft,
              playerSquareBottom,
              (playerSquareLeft) + UI_FULLPLAYER_DIM_HORIZ,
              (playerSquareBottom) + UI_FULLPLAYER_DIM_VERT);

    /*Draw mobs*/
    while(currentMob != NULL){
        mobSquareLeft = UI_FULLMAP_RIGHT - ((int)currentMob->x_pos - LEFTWALL + 2)*UI_FULLSQUARE_DIM_HORIZ;
        mobSquareBottom = UI_FULLMAP_BOTTOM + ((int)currentMob->z_pos - BOTTOMWALL - 1)*UI_FULLSQUARE_DIM_VERT;
        if(currentMob->type == ORBITER){
            set2Dcolour(orbiterColour);
            draw2Dbox(mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ),
                      mobSquareBottom + (UI_FULLSQUARE_DIM_HORIZ),
                      mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ*2),
                      mobSquareBottom + (UI_FULLSQUARE_DIM_VERT*2));
            set2Dcolour(black);
            draw2Dbox(mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ*0.5),
                      mobSquareBottom + (UI_FULLSQUARE_DIM_HORIZ*0.5),
                      mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ*2.6),
                      mobSquareBottom + (UI_FULLSQUARE_DIM_VERT*2.6));
            set2Dcolour(orbiterRingColour);
            draw2Dbox(mobSquareLeft,
                      mobSquareBottom,
                      mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ*3),
                      mobSquareBottom + (UI_FULLSQUARE_DIM_VERT*3));
        }
        else{
            set2Dcolour(pulsarColour);
            draw2Dline(mobSquareLeft,
                       mobSquareBottom,
                       mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ*3),
                       mobSquareBottom + (UI_FULLSQUARE_DIM_VERT*3),
                       ((UI_FULLSQUARE_DIM_HORIZ + UI_FULLSQUARE_DIM_VERT)) / (2 * 2));
            draw2Dline(mobSquareLeft,
                       mobSquareBottom + (UI_FULLSQUARE_DIM_VERT*3),
                       mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ*3),
                       mobSquareBottom,
                       ((UI_FULLSQUARE_DIM_HORIZ + UI_FULLSQUARE_DIM_VERT)) / (2 * 2));
            set2Dcolour(black);
            draw2Dbox(mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ*0.9),
                      mobSquareBottom + (UI_FULLSQUARE_DIM_HORIZ*0.9),
                      mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ*2.1),
                      mobSquareBottom + (UI_FULLSQUARE_DIM_VERT*2.1));
            set2Dcolour(pulsarColour);
            draw2Dbox(mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ*0.6),
                      mobSquareBottom + (UI_FULLSQUARE_DIM_HORIZ*0.6),
                      mobSquareLeft + (UI_FULLSQUARE_DIM_HORIZ*2.3),
                      mobSquareBottom + (UI_FULLSQUARE_DIM_VERT*2.3));
        }
        currentMob = currentMob->next;
    }


    /*Draw projectiles*/
    for(i = 0; i < BULLET_ARRAY_SIZE; i++){
        if(bulletArray[i].existsInWorld == 1){
            bulletSquareLeft = UI_FULLMAP_RIGHT - (bulletArray[i].x_pos - LEFTWALL - 1)*UI_FULLSQUARE_DIM_HORIZ;
            bulletSquareBottom = UI_FULLMAP_BOTTOM + (bulletArray[i].z_pos - BOTTOMWALL + 1)*UI_FULLSQUARE_DIM_VERT;

            set2Dcolour(bulletColour);
            draw2Dbox(bulletSquareLeft,
                      bulletSquareBottom,
                      (bulletSquareLeft) + UI_BULLET_DIM_HORIZ,
                      (bulletSquareBottom) + UI_BULLET_DIM_VERT);
        }
    }


    set2Dcolour(borderColour);
    /*Draw the minimap border - left wall, bottom wall, right wall, top wall*/
    draw2Dline(UI_FULLMAP_LEFT - (UI_FULLMAP_BORDER_WIDTH / 2),
               UI_FULLMAP_BOTTOM - UI_FULLMAP_BORDER_WIDTH,
               UI_FULLMAP_LEFT - (UI_FULLMAP_BORDER_WIDTH / 2),
               UI_FULLMAP_TOP + UI_FULLMAP_BORDER_WIDTH + UI_FULLSQUARE_DIM_VERT,
               UI_FULLMAP_BORDER_WIDTH);

    draw2Dline(UI_FULLMAP_LEFT,
               UI_FULLMAP_BOTTOM - (UI_FULLMAP_BORDER_WIDTH / 2) - 1,
               UI_FULLMAP_RIGHT + UI_FULLMAP_BORDER_WIDTH + UI_FULLSQUARE_DIM_HORIZ,
               UI_FULLMAP_BOTTOM - (UI_FULLMAP_BORDER_WIDTH / 2) - 1,
               UI_FULLMAP_BORDER_WIDTH);

    draw2Dline(UI_FULLMAP_RIGHT + (UI_FULLMAP_BORDER_WIDTH / 2) + UI_FULLSQUARE_DIM_HORIZ + 1,
               UI_FULLMAP_BOTTOM - UI_FULLMAP_BORDER_WIDTH,
               UI_FULLMAP_RIGHT + (UI_FULLMAP_BORDER_WIDTH / 2) + UI_FULLSQUARE_DIM_HORIZ + 1,
               UI_FULLMAP_TOP + UI_FULLMAP_BORDER_WIDTH + UI_FULLSQUARE_DIM_VERT,
               UI_FULLMAP_BORDER_WIDTH);

    draw2Dline(UI_FULLMAP_LEFT,
               UI_FULLMAP_TOP + (UI_FULLMAP_BORDER_WIDTH / 2) + UI_FULLSQUARE_DIM_VERT,
               UI_FULLMAP_RIGHT + UI_FULLSQUARE_DIM_HORIZ,
               UI_FULLMAP_TOP + (UI_FULLMAP_BORDER_WIDTH / 2) + UI_FULLSQUARE_DIM_VERT,
               UI_FULLMAP_BORDER_WIDTH);

    for(i = LEFTWALL; i <= RIGHTWALL; i++){
        for(j = FLOORHEIGHT+WALL_HEIGHT; j >= FLOORHEIGHT; j--){
            /*Draw the map from top to bottom to prevent overlap (y)*/

            currentBufferValue = j - FLOORHEIGHT + 1;
            for(k = BOTTOMWALL; k <= TOPWALL; k++){
                if((world[i][j][k] != 0) && (ui_Ybuffer[i][k] < currentBufferValue)){
                    /*Select a colour to match the world's cube colour*/
                    switch(world[i][j][k]){
                        case 2:
                            /*Blue*/
                            set2Dcolour(blue);
                            break;
                        case 4:
                            /*Black*/
                            set2Dcolour(black);
                            break;
                        case 5:
                            /*White*/
                            set2Dcolour(white);
                            break;
                        case 6:
                            /*Purple*/
                            set2Dcolour(purple);
                            break;
                        case 9:
                            /*Light Blue*/
                            set2Dcolour(lightBlue);
                            break;
                        case 11:
                            set2Dcolour(pickupRed);
                            break;
                        case 12:
                            set2Dcolour(pickupGreen);
                            break;
                        case 13:
                            set2Dcolour(pickupBlue);
                            break;
                        default:
                            /*Black*/
                            set2Dcolour(black);
                            break;
                    }

                    /*Draw from right to left, bottom to top*/
                    currentSquareLeft = UI_FULLMAP_RIGHT - (i - LEFTWALL)*UI_FULLSQUARE_DIM_HORIZ;
                    currentSquareBottom = UI_FULLMAP_BOTTOM + (k - BOTTOMWALL)*UI_FULLSQUARE_DIM_VERT;

                    draw2Dbox((currentSquareLeft),
                              (currentSquareBottom),
                              (currentSquareLeft) + UI_FULLSQUARE_DIM_HORIZ,
                              (currentSquareBottom) + UI_FULLSQUARE_DIM_VERT);

                    /*Key Highlight
                    if((world[i][j][k] == 5) && (j <= FLOORHEIGHT+2)){
                        set2Dcolour(yellow);
                        draw2Dbox((currentSquareLeft - 2),
                                (currentSquareBottom - 2),
                                (currentSquareLeft) + UI_SQUARE_DIM_HORIZ + 4,
                                (currentSquareBottom) + UI_SQUARE_DIM_VERT + 4);
                        set2Dcolour(white);
                    }*/

                    ui_Ybuffer[i][k] = currentBufferValue;
                }
            }
        }
    }
}

void drawKey(){
    set2Dcolour(black);
    draw2Dbox(UI_KEY_BOX_LEFT + (5 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_BOTTOM + (7 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_LEFT + (6 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_BOTTOM + (8 * UI_KEY_PIXEL_SIZE));
    draw2Dbox(UI_KEY_BOX_LEFT + (6 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_BOTTOM + (3 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_LEFT + (7 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_BOTTOM + (4 * UI_KEY_PIXEL_SIZE));
    set2Dcolour(white);
    draw2Dbox(UI_KEY_BOX_LEFT + (4 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_BOTTOM + (6 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_LEFT + (7 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_BOTTOM + (9 * UI_KEY_PIXEL_SIZE));
    draw2Dbox(UI_KEY_BOX_LEFT + (5 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_BOTTOM + (5 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_LEFT + (6 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_BOTTOM + (6 * UI_KEY_PIXEL_SIZE));
    draw2Dbox(UI_KEY_BOX_LEFT + (5 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_BOTTOM + (2 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_LEFT + (7 * UI_KEY_PIXEL_SIZE), UI_KEY_BOX_BOTTOM + (5 * UI_KEY_PIXEL_SIZE));

}


void drawGameplayUI(){
    /*Draw the key if it has been acquired*/
    if(playerHasKey == TRUE){
        drawKey();
    }

    /*Draw the box containing the key*/
    set2Dcolour(yellow);
    draw2Dline(UI_KEY_BOX_RIGHT, UI_KEY_BOX_TOP, UI_KEY_BOX_LEFT, UI_KEY_BOX_TOP, UI_KEY_BORDER_THICC);
    draw2Dline(UI_KEY_BOX_RIGHT, UI_KEY_BOX_BOTTOM, UI_KEY_BOX_LEFT, UI_KEY_BOX_BOTTOM, UI_KEY_BORDER_THICC);
    draw2Dline(UI_KEY_BOX_LEFT, UI_KEY_BOX_TOP, UI_KEY_BOX_LEFT, UI_KEY_BOX_BOTTOM, UI_KEY_BORDER_THICC);
    draw2Dline(UI_KEY_BOX_RIGHT, UI_KEY_BOX_TOP, UI_KEY_BOX_RIGHT, UI_KEY_BOX_BOTTOM, UI_KEY_BORDER_THICC);

    set2Dcolour(black);
    draw2Dbox(UI_KEY_BOX_LEFT, UI_KEY_BOX_BOTTOM, UI_KEY_BOX_RIGHT, UI_KEY_BOX_TOP);

    /*Draw hit indicator*/
    hitIndicatorColour[3] = 0.5 - (0.5 / ((float)playerInvincibilityTimer + 1.0));
    set2Dcolour(transparentColour);
    draw2Dbox((screenWidth / 10) * 3,
              (screenHeight / 10) * 3,
              (screenWidth / 10) * 7,
              (screenHeight / 10) * 7);

    set2Dcolour(hitIndicatorColour);
    draw2Dbox((screenWidth / 10) * 2,
              (screenHeight / 10) * 2,
              (screenWidth / 10) * 8,
              (screenHeight / 10) * 8);
}
