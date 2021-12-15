#ifndef SIMULATION_RANDOMTREE_H
#define SIMULATION_RANDOMTREE_H

#include "../../Include/Visualization/Grid/Grid.h"
#include <vector>

/* all available states of a cell in the grid
*/
typedef enum{
    FREE, 
    OBSTACLE,
    NODE, /* cells added as part of random tree
          */ 
    NODE_CONNECTION, /* cells connection 2 nodes or one node
                      * and start/end cell
                     */
    START_CELL, 
    END_CELL
}cellState;

/* this is used to determine the method to increase the
 * width of obstacel stream
*/
typedef enum{
    BOTTOM, 
    RIGHT, 
    TOP, 
    LEFT,
    OTHER
}widthType;

class RandomTreeClass: public GridClass{
    private:
        /* This will be the NxN grid that we will be working on
        */
        int *cellCurr;
        /* this determines the next cell to set as NODE at a distance
         * along the line connected to random cell
        */
        int step;
        /* final distance between start and end cells connected via
         * the computer path
        */
        int finalDistance;
        /* number of nodes/cells added
        */
        int numCellsAdded;
        /* start and end goal cell position
        */
        int startX, startY;
        int endX, endY;
        /* set a bigger end goal radius
        */
        int endCellWidth;
        /* border (obstacle) width
        */
        int borderWidth;
        /* other visual params
        */
        int otherCellHighlightWidth, endCellHighlightWidth;
        float highlightAlpha;

        /* util functions
        */
        int getIdx(int i, int j);
        bool isCellFree(int i, int j);
        bool isCellNode(int i, int j);
        void setCellColorFromState(int i, int j, cellState state, float alpha = 1.0);
        void setCellAsFree(int i, int j);
        void setCellBlockAsFree(int i, int j, int width);
        void setCellAsObstacle(int i, int j);
        void setCellAsNode(int i, int j);
        void setCellAsNodeConnection(int i, int j);
        void setCellAsStartCell(int i, int j);
        void setCellAsEndCell(int i, int j);
        void highlightCell(int i, int j, cellState state);
        void deHighlightCell(int i, int j);
        
        /* primary functions
        */
        void getRandomCell(int &i, int &j);
        void getDistanceBetweenCells(int i1, int j1, int i2, int j2);
        std::vector<std::pair<int, int> > connectTwoCells(int i1, int j1, int i2, int j2);
        void setCellAsObstacleStream(int i1, int j1, int i2, int j2, const int width, 
                                     widthType wType);
        void setCellAsNodeConnectionStream(int i1, int j1, int i2, int j2);


    public:
        RandomTreeClass(int _step, int _N, int _scale, bool noStroke);
        ~RandomTreeClass(void);

        /* override functions
        */
        void setObstacleCells(void);
        void setStartAndEndCells(void);
        void simulationStep(void);
};
#endif /* SIMULATION_RANDOMTREE_H
*/
