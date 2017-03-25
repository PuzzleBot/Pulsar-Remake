#include "graphics.h"

Parabola createParabola(double xStart, double yStart, double zStart, double xEnd, double yEnd, double zEnd, double height){
    Parabola parabola;

    parabola.vertexHeight = height;

    parabola.x_start = xStart;
    parabola.y_start = yStart;
    parabola.z_start = zStart;

    parabola.x_end = xEnd;
    parabola.y_end = yEnd;
    parabola.z_end = zEnd;

    parabola.xzDistance = sqrt(pow(xEnd - xStart, 2) + pow(zEnd - zStart, 2));

    parabola.currentTotalStepLength = 0.01;

    return(parabola);
}

double parabolaStep(Parabola * parabola, double stepSize){
    parabola->currentTotalStepLength = parabola->currentTotalStepLength + stepSize;
    if(parabola->currentTotalStepLength >= parabola->xzDistance){
        parabola->currentTotalStepLength = parabola->xzDistance - 0.01;
    }
    printf("%.2f\n", parabola->currentTotalStepLength);
    return calculateHeightAtCurrent(parabola);
}

double calculateHeightAtCurrent(Parabola * parabola){
    /*y = h(-d/2)^2 * x * (x - dest)
      Parabola with roots at the start and end xz values, and a set vertex height
    */
    double trueVertexHeight = parabola->vertexHeight - FLOORHEIGHT + 1;
    return -((trueVertexHeight * pow(-parabola->xzDistance / 2, -2)) * ((parabola->currentTotalStepLength) * (parabola->currentTotalStepLength - parabola->xzDistance))) + FLOORHEIGHT + 1;
}
