#include "../../Include/Simulation/RandomTree.h"
#include "../../Include/Utils/Common.h"
#include <random>
#include <cmath> /* for for round(), pow(). sqrt()
*/

int RandomTreeClass::getIdx(int i, int j){
    return (i + (j * N));
}

bool RandomTreeClass::isCellFree(int i, int j){
    return cellCurr[getIdx(i, j)] == FREE;
}

bool RandomTreeClass::isCellObstacle(int i, int j){
    return cellCurr[getIdx(i, j)] == OBSTACLE;
}

bool RandomTreeClass::isCellEndCell(int i, int j){
    return cellCurr[getIdx(i, j)] == END_CELL;
}

void RandomTreeClass::setCellColorFromState(int i, int j, cellState state, float alpha){
    colorVal cVal = state == FREE ? whiteVal : state == OBSTACLE ? blackVal :
                    state == NODE ? blueVal : state == START_CELL ? greenVal :
                    state == NODE_CONNECTION ? blueVal : redVal;

    genCellColor(i, j, cVal, alpha);
}

void RandomTreeClass::setCellBlockToState(int i, int j, cellState state, int width){
    int r, c;
    for(r = -width; r <= width; r++){
        for(c = -width; c <= width; c++){
            /* boundary guards
            */
            if((i + r < 0) || (i + r > N-1))
                continue;
            if((j + c < 0) || (j + c > N-1))
                continue;
            
            /* only operate on free cells, when we are setting
             * states, but when we are trying to free only 
             * operate on end cell
             * NOTE: This fn is used only to free end cell block
            */
            if((isCellFree(i + r, j + c) && state != FREE) || 
            (isCellEndCell(i + r, j + c) && state == FREE)){
                cellCurr[getIdx(i + r, j + c)] = state;
                /* set color according to cell state
                */
                setCellColorFromState(i + r, j + c, state);   
            }        
        }
    }
}

void RandomTreeClass::setCellAsFree(int i, int j){
    cellCurr[getIdx(i, j)] = FREE;
    /* set color according to cell state
    */
    setCellColorFromState(i, j, FREE);
}

/* Set width x width cells as free
 * NOTE: this op has to be performed with (i,j) as center of
 * the block
*/
void RandomTreeClass::setCellBlockAsFree(int i, int j, int width){
    setCellBlockToState(i, j, FREE, width);
}

void RandomTreeClass::setCellAsObstacle(int i, int j){
    cellCurr[getIdx(i, j)] = OBSTACLE;
    /* set color according to cell state
    */
    setCellColorFromState(i, j, OBSTACLE);
}

void RandomTreeClass::setCellAsNode(int i, int j){
    cellCurr[getIdx(i, j)] = NODE;
    /* set color according to cell state
    */
    setCellColorFromState(i, j, NODE);
}

void RandomTreeClass::setCellAsNodeConnection(int i, int j){
    cellCurr[getIdx(i, j)] = NODE_CONNECTION;
    /* set color according to cell state
    */
    setCellColorFromState(i, j, NODE_CONNECTION, nodeConnectionAlpha);
}

/* different from other set functions, it saves the value
*/ 
void RandomTreeClass::setCellAsStartCell(int i, int j){
    /* save values
    */
    startX = i;
    startY = j;

    cellCurr[getIdx(i, j)] = START_CELL;
    /* set color according to cell state
    */
    setCellColorFromState(i, j, START_CELL);
}

/* different from other set functions, it saves the value, and
 * the width is higher for an end cell
*/ 
void RandomTreeClass::setCellAsEndCell(int i, int j){
    /* save values
    */
    endX = i;
    endY = j;

    setCellBlockToState(i, j, END_CELL, endCellWidth);
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
    /* returns set of points between (i1,j1) and (i2,j2) included
    */
    points = connectTwoCells(i1, j1, i2, j2);

    for(int i = 0; i < points.size(); i++){
        int px = points[i].first;
        int py = points[i].second;
        /* NOTE: we need to exclude the points (i1,j1) and (i2,j2) from
         * points vector
        */
        if((px == i1 && py == j1) || (px == i2 && py == j1))
            continue;
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
        /* NOTE: we need to exclude the points (i1,j1) and (i2,j2) from
         * points vector
        */
        if((px == i1 && py == j1) || (px == i2 && py == j1))
            continue;
        setCellAsFree(px, py); 
    }
}

/* return cell coordinates between (i1,j1) and (i2,j2), both end
 * points included
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
    double slope = dx == 0 ? INT_MAX : dy/dx;
    /* generate numPoints between the two points
    */
    int numPoints = N;
    double px, py;
    for(double i = 0; i < numPoints; i++){
        py = slope == 0 ? 0 : dy * (i / numPoints);
        if(slope != INT_MAX)
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

/* we don't change the cellCurr value for the highlighted cells,
 * instead we only change the color. This is just to improve 
 * visibility
*/
void RandomTreeClass::highlightCell(int i, int j, cellState state){
    /* set highlight width
     * NOTE: end goal cell has a higher width, so highlight width 
     * should be higher as well
    */
    int width = state == START_CELL? 
                otherCellHighlightWidth : endCellHighlightWidth;
    /* this function is also use to de-highlight cells by passing in 
     * state as FREE, so we need the alpha in this case to be 1.0
    */
    float alpha = state == FREE ? 1.0 : highlightAlpha;

    for(int r = -width; r <= width; r++){
        for(int c = -width; c <= width; c++){
            /* boundary guards
            */
            if((i + r < 0) || (i + r > N-1))
                continue;
            if((j + c < 0) || (j + c > N-1))
                continue;
            /* only highlight over free cells, i.e we don;t corrupt
             * cell states
            */
            if(isCellFree(i + r, j + c))
                setCellColorFromState(i + r, j + c, state, alpha);         
        }
    }
}

/* path contains cell/node coords
*/
void RandomTreeClass::highlightPath(std::vector<std::pair<int, int>> path, cellState state){
    int width = pathHighlightWidth;
    float alpha = state == FREE ? 1.0 : pathHighlightAlpha;

    int n = path.size();
    /* something wrong if the path has only node coord
    */
    if(n < 2)
        assert(false);

    int i = n - 1;
    while(i >= 1){
        int p1x = path[i].first;
        int p1y = path[i].second;
        i--;

        int p2x = path[i].first;
        int p2y = path[i].second;

        std::vector<std::pair<int, int> > points = connectTwoCells(p1x, p1y, p2x, p2y);
        for(int k = 0; k < points.size(); k++){
            for(int r = -width; r <= width; r++){
                for(int c = -width; c<= width; c++){
                    /* boundary guards
                    */
                    if(points[k].first + r < 0 || points[k].first + r > N-1)
                        continue;
                    if(points[k].second + c < 0 || points[k].second + c > N-1)
                        continue;

                    /* highlight only over free cells, and in the x direction only
                    */
                    if(isCellFree(points[k].first + r, points[k].second + c))
                        setCellColorFromState(points[k].first + r, points[k].second + c, state, alpha); 
                }
            }
        }
    }
}

void RandomTreeClass::deHighlightCell(int i, int j){
    /* dehighlight by setting color as FREE and alpha to 1.0
    */
    highlightCell(i, j, FREE);
}

void RandomTreeClass::deHighlightPath(std::vector<std::pair<int, int>> path){
    highlightPath(path, FREE);
}

void RandomTreeClass::restartRenderLoop(void){
    readyToStart = false;
    endCellSet = false;
    /* NOTE: we cannot reset the start cell since the tree has
     * already grown from the root node, if we were to set another
     * node as a root node, then there exist a node who doesn't have
     * a parent - which conflicts the idea of a tree
    */
    startCellSet = true;
    /* clear output boolean and other vars
    */
    pathFound = false;
}

/* given a start and an end range, generate a random number.
*/
int RandomTreeClass::getRandomAmount(int start, int end){
    /* At first, the std::random_device object should be 
     * initialized. It produces non-deterministic random bits 
     * for random engine seeding, which is crucial to avoid 
     * producing the same number sequences. Here we use std::
     * default_random_engine to generate pseudo-random values, 
     * but you can declare specific algorithm engine. Next, we
     * initialize a uniform distribution and pass min/max values
     * as optional arguments.
    */
    std::random_device rd;
    std::default_random_engine eng(rd());
    std::uniform_real_distribution<> distr(start, end);
    return distr(eng);
}

float RandomTreeClass::getDistanceBetweenCells(int i1, int j1, int i2, int j2){
    return sqrt(pow((j2 - j1), 2) + pow((i2 - i1), 2));
}