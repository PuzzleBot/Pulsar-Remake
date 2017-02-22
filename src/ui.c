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

extern Bullet bulletArray[MAX_BULLETS];

GLfloat blue[] = {0.0, 0.0, 1.0, 0.5};
GLfloat purple[] = {1.0, 0.0, 1.0, 0.5};
GLfloat black[] = {0.0, 0.0, 0.0, 0.5};
GLfloat white[] = {1.0, 1.0, 1.0, 0.5};

GLfloat borderColour[] = {0.0, 0.0, 0.0, 1.0};
GLfloat playerColour[] = {1.0, 1.0, 0.0, 1.0};
GLfloat bulletColour[] = {0.0, 1.0, 1.0, 1.0};

void drawSmallMinimap(){
    float vpX, vpY, vpZ;
    
    int i, j, k;
    
    int currentBufferValue = 0;
    
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
    
    
    /*Draw the player*/
    getViewPosition(&vpX, &vpY, &vpZ);
    vpX = -vpX;
    vpY = -vpY;
    vpZ = -vpZ;
    
    playerSquareLeft = UI_MAP_LEFT + (vpX - LEFTWALL)*UI_SQUARE_DIM_HORIZ;
    playerSquareBottom = UI_MAP_BOTTOM + (vpZ - BOTTOMWALL)*UI_SQUARE_DIM_VERT;
    
    set2Dcolour(playerColour);
    draw2Dbox(playerSquareLeft,
              playerSquareBottom,
              (playerSquareLeft) + UI_PLAYER_DIM_HORIZ,
              (playerSquareBottom) + UI_PLAYER_DIM_VERT);
    
    
    /*Draw projectiles*/
    for(i = 0; i < MAX_BULLETS; i++){
        if(bulletArray[i].existsInWorld == 1){
            bulletSquareLeft = UI_MAP_LEFT + (bulletArray[i].x_pos - LEFTWALL)*UI_SQUARE_DIM_HORIZ;
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
        for(j = FLOORHEIGHT+3; j >= FLOORHEIGHT; j--){
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
                        default:
                            /*Black*/
                            set2Dcolour(black);
                            break;
                    }
                    
                    /*Draw from left to right, bottom to top*/
                    currentSquareLeft = UI_MAP_LEFT + (i - LEFTWALL)*UI_SQUARE_DIM_HORIZ;
                    currentSquareBottom = UI_MAP_BOTTOM + (k - BOTTOMWALL)*UI_SQUARE_DIM_VERT;
                    
                    draw2Dbox((currentSquareLeft),
                              (currentSquareBottom),
                              (currentSquareLeft) + UI_SQUARE_DIM_HORIZ,
                              (currentSquareBottom) + UI_SQUARE_DIM_VERT);
                    
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
    
    /*Draw the player*/
    getViewPosition(&vpX, &vpY, &vpZ);
    vpX = -vpX;
    vpY = -vpY;
    vpZ = -vpZ;
    
    playerSquareLeft = UI_FULLMAP_LEFT + (vpX - LEFTWALL)*UI_FULLSQUARE_DIM_HORIZ;
    playerSquareBottom = UI_FULLMAP_BOTTOM + (vpZ - BOTTOMWALL)*UI_FULLSQUARE_DIM_VERT;
    
    set2Dcolour(playerColour);
    draw2Dbox(playerSquareLeft,
              playerSquareBottom,
              (playerSquareLeft) + UI_FULLPLAYER_DIM_HORIZ,
              (playerSquareBottom) + UI_FULLPLAYER_DIM_VERT);
    
    
    /*Draw projectiles*/
    for(i = 0; i < MAX_BULLETS; i++){
        if(bulletArray[i].existsInWorld == 1){
            bulletSquareLeft = UI_FULLMAP_LEFT + (bulletArray[i].x_pos - LEFTWALL)*UI_FULLSQUARE_DIM_HORIZ;
            bulletSquareBottom = UI_FULLMAP_BOTTOM + (bulletArray[i].z_pos - BOTTOMWALL)*UI_FULLSQUARE_DIM_VERT;
            
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
        for(j = FLOORHEIGHT+3; j >= FLOORHEIGHT; j--){
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
                        default:
                            /*Black*/
                            set2Dcolour(black);
                            break;
                    }
                    
                    /*Draw from left to right, bottom to top*/
                    currentSquareLeft = UI_FULLMAP_LEFT + (i - LEFTWALL)*UI_FULLSQUARE_DIM_HORIZ;
                    currentSquareBottom = UI_FULLMAP_BOTTOM + (k - BOTTOMWALL)*UI_FULLSQUARE_DIM_VERT;
                    
                    draw2Dbox((currentSquareLeft),
                              (currentSquareBottom),
                              (currentSquareLeft) + UI_FULLSQUARE_DIM_HORIZ,
                              (currentSquareBottom) + UI_FULLSQUARE_DIM_VERT);
                    
                    ui_Ybuffer[i][k] = currentBufferValue;
                }
            }
        }
    }
}
