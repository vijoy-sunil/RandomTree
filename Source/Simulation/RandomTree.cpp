#include "../../Include/Simulation/RandomTree.h"
#include "../../Include/Simulation/Constants.h"
#include "../../Include/Utils/Common.h"
#include <iostream>
#include <cmath> /* for for round()
*/

RandomTreeClass::RandomTreeClass(int _step, int _N, int _scale, bool noStroke): 
GridClass(_N, _scale, noStroke){
    cellCurr = (int*)calloc(N * N, sizeof(int));
    step = _step;

    finalDistance = 0;
    numCellsAdded = 0;

    startX = 0; startY = 0;
    endX = 0; endY = 0;

    endCellWidth = 0.02 * N;
    borderWidth = 0.03 * N;
    /* this highlight width is used for START_CELL or NODE
    */
    otherCellHighlightWidth = 0.01 * N;
    /* this highlight width is used for END_CELL
    */
    endCellHighlightWidth = 0.03 * N;
    /* used for START_CELL or END_CELL highlight alpha, for other
     * states it will be 1.0
    */
    highlightAlpha = 0.3;
}

RandomTreeClass::~RandomTreeClass(void){
    free(cellCurr);
}

void RandomTreeClass::getRandomCell(int &i, int &j){

}

void RandomTreeClass::getDistanceBetweenCells(int i1, int j1, int i2, int j2){

}

/* place node 1 step away from source node along the line
 * connecting source node and random node at (i,j)
*/
void RandomTreeClass::placeNode(int i, int j){

}

/* check if (i2,j2) is within step distance of (i1,j1)
*/
bool RandomTreeClass::isWithinStepDistance(int i1, int j1, int i2, int j2){

}

/* check if (i2,j2) is within neighborood distance of (i1,j1)
*/
bool RandomTreeClass::isWithinNeighborhoodDistance(int i1, int j1, int i2, int j2){

}


/* return cell coordinates between (i1,j1) and (i2,j2)
*/
std::vector<std::pair<int, int> > RandomTreeClass::connectTwoCells
(int i1, int j1, int i2, int j2){
    std::vector<std::pair<int, int> > points;
    /* get slope
    */
    double dy = j2 - j1;
    double dx = i2 - i1;
    /* division by zero guard
    */
    double slope = dx == 0 ? -1 : dy/dx;
    /* generate numPoints between the two points
    */
    int numPoints = N;
    double px, py;
    for(double i = 0; i < numPoints; i++){
        py = slope == 0 ? 0 : dy * (i / numPoints);
        if(slope != -1)
            px = slope == 0 ? dx * (i / numPoints) : py / slope;
        else
            px = 0;

        points.push_back(std::make_pair((int)round(px) + i1, (int)round(py) + j1));
    }
    /* do not forget to add the destination point as well (i2, j2)
    */
    points.push_back(std::make_pair(i2, j2));
    return points;
}

/* used to create line obstacles (walls), this also takes
 * in the width parameter
*/
void RandomTreeClass::setCellAsObstacleStream(int i1, int j1, int i2, int j2, const int width, 
                                              widthType wType){
    std::vector<std::pair<int, int> > points;
    points = connectTwoCells(i1, j1, i2, j2);

    int j, px, py;
    for(int i = 0; i < points.size(); i++){
        /* increment width
        */
        for(j = 0; j < width; j++){
            px = points[i].first;
            py = points[i].second;

            px = wType == BOTTOM ? px : wType == RIGHT ? px - j : wType == LEFT ? px + j : px;
            py = wType == RIGHT ? py : wType == TOP ? py - j : wType == BOTTOM ? py + j : py;
            setCellAsObstacle(px, py);
        }
    }
}

void RandomTreeClass::setCellAsNodeConnectionStream(int i1, int j1, int i2, int j2){
    std::vector<std::pair<int, int> > points;
    points = connectTwoCells(i1, j1, i2, j2);

    for(int i = 0; i < points.size(); i++){
        int px = points[i].first;
        int py = points[i].second;
        setCellAsNodeConnection(px, py); 
    }
}

/* remove path between two nodes
*/
void RandomTreeClass::clearNodeConnectionStream(int i1, int j1, int i2, int j2){
    std::vector<std::pair<int, int> > points;
    points = connectTwoCells(i1, j1, i2, j2);

    for(int i = 0; i < points.size(); i++){
        int px = points[i].first;
        int py = points[i].second;
        setCellAsFree(px, py); 
    }
}

void RandomTreeClass::setObstacleCells(void){
    setCellAsObstacleStream(0, 0, N-1, 0, borderWidth, BOTTOM);
    setCellAsObstacleStream(N-1, 0, N-1, N-1, borderWidth, RIGHT);
    setCellAsObstacleStream(0, N-1, N-1, N-1, borderWidth, TOP);
    setCellAsObstacleStream(0, 0, 0, N-1, borderWidth, LEFT);

#if PREDEF_OBSTACLE == 1
    /* set additional walls as obstacles
    */    
    setCellAsObstacleStream(N/3, 0, N/3, N/3, borderWidth, LEFT);
    setCellAsObstacleStream(N/3, N/2, N/3, N-1, borderWidth, LEFT);
    setCellAsObstacleStream(N/1.5, 0, N/1.5, N/1.5, borderWidth, LEFT);  
    setCellAsObstacleStream(N-1, 0.8 * N, N/1.5, 0.8 * N, borderWidth, BOTTOM); 
#endif
#if RANDOM_OBSTACLE == 1

#endif
}

void RandomTreeClass::setStartAndEndCells(void){
    /* startCellSet is set to true in the process input function by
     * user input. This confirms the start cell position and moves on
     * to setting the end cell position
    */
    if(!readyToStart){
        if(!startCellSet && mouseClicked){
            mouseClicked = false;
            mouseAction(xPos, yPos);

            if(isCellFree(cellX, cellY)){
                /* clear previous selection
                */
                setCellAsFree(startX, startY);
                /* remove highlight
                */
                deHighlightCell(startX, startY);
                /* default to origin
                */ 
                startX = 0;
                startY = 0;

                setCellAsStartCell(cellX, cellY);   
                /* highlight cell
                */
                highlightCell(startX, startY, START_CELL);
            } 
        }
        /* Now set the end goal cell, startCellSet is set to true in the
         * process input function. endCellSet is set to true in the process
         * input function to confirm the input selection
        */
        if(startCellSet && !endCellSet && mouseClicked){
            mouseClicked = false;
            mouseAction(xPos, yPos);

            if(isCellFree(cellX, cellY)){
               /* clear previous selection, this is a wrapper around 
                 * setCellAsFree
                */
                setCellBlockAsFree(endX, endY, endCellWidth); 
                /* remove highlight
                */
                deHighlightCell(endX, endY);  
                /* default to origin
                */ 
                endX = 0;
                endY = 0;   

                setCellAsEndCell(cellX, cellY); 
                /* highlight cell
                */ 
                highlightCell(endX, endY, END_CELL);
            }         
        }

        /* After both start and end cells are set, we can set readyToStart to
         * true to start the simulation
        */
        if(startCellSet && endCellSet){
            std::cout<<"START CELL: "<<startX<<","<<startY
                     <<" END CELL: "<<endX<<","<<endY<<std::endl;
            readyToStart = true;
        }
    }
}

void RandomTreeClass::simulationStep(void){
    /* this is set to false via reset or if start and end cells have
     * not been selected
    */
    if(readyToStart){

    }
}