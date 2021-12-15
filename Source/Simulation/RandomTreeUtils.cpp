#include "../../Include/Simulation/RandomTree.h"

int RandomTreeClass::getIdx(int i, int j){
    return (i + (j * N));
}

bool RandomTreeClass::isCellFree(int i, int j){
    return cellCurr[getIdx(i, j)] == FREE;
}

/* check if the cell is a node, or node connection, start cell
 * or even an end cell
*/
bool RandomTreeClass::isCellNode(int i, int j){
    return cellCurr[getIdx(i, j)] >= NODE;
}

void RandomTreeClass::setCellColorFromState(int i, int j, cellState state, float alpha){
    colorVal cVal = state == FREE ? whiteVal : state == OBSTACLE ? blackVal :
                    state == NODE ? blueVal : state == START_CELL ? greenVal :
                    state == NODE_CONNECTION ? blueVal : redVal;

    genCellColor(i, j, cVal, alpha);
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
    int r, c;
    for(r = -width; r <= width; r++){
        for(c = -width; c <= width; c++){
            /* skip obstacles, clear everything else
            */
            if(isCellNode(i + r, j + c)){
                cellCurr[getIdx(i + r, j + c)] = FREE;
                /* set color according to cell state
                */
                setCellColorFromState(i + r, j + c, FREE);   
            }        
        }
    }
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
    setCellColorFromState(i, j, NODE_CONNECTION);
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

    int r, c;
    for(r = -endCellWidth; r <= endCellWidth; r++){
        for(c = -endCellWidth; c <= endCellWidth; c++){
            if(isCellFree(i + r, j + c)){
                cellCurr[getIdx(i + r, j + c)] = END_CELL;
                /* set color according to cell state
                */
                setCellColorFromState(i + r, j + c, END_CELL);   
            }        
        }
    }
}

/* we don't change the cellCurr value for the highlighted cells,
 * instead we only change the color. This is just to improve 
 * visibility for START_CELL, END_CELL and NODE
*/
void RandomTreeClass::highlightCell(int i, int j, cellState state){
    /* set highlight width
     * NOTE: end goal cell has a higher width, so highlight width 
     * should be higher as well
    */
    int width = (state == START_CELL || state == NODE) ? 
                 otherCellHighlightWidth : endCellHighlightWidth;
    /* this function is also use to de-highlight cells by passing in 
     * state as FREE, so we need the alpha in this case to be 1.0
    */
    float alpha = (state == FREE || state == NODE) ? 1.0 : highlightAlpha;

    for(int r = -width; r <= width; r++){
        for(int c = -width; c <= width; c++){
            /* don't highligh over obstacles or the focus cell itself
            */
            if(isCellFree(i + r, j + c))
                setCellColorFromState(i + r, j + c, state, alpha);         
        }
    }
}

void RandomTreeClass::deHighlightCell(int i, int j){
    /* dehighlight by setting color as FREE and alpha to 1.0
    */
    highlightCell(i, j, FREE);
}
