#ifndef SIMULATION_RANDOMTREE_H
#define SIMULATION_RANDOMTREE_H

#include "../../Include/Visualization/Grid/Grid.h"
#include "../../Include/Utils/Tree.h"
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

class RandomTreeClass: public GridClass, public TreeClass{
    private:
        /* This will be the NxN grid that we will be working on
        */
        int *cellCurr;
        /* this determines the next cell to set as NODE at a distance
         * along the line connected to random cell
        */
        int step;
        /* the nieghborhood distance to rearrange the min cost path
        */
        int neighborhood;
        /* number of nodes/cells added
        */
        int numNodesAdded;
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
        /* final output boolean
        */
        bool pathFound;
        /* holds node coords from end cell to start cell
        */
        std::vector<std::pair<int, int>> path;
        /* other visual params
        */
        int otherCellHighlightWidth, endCellHighlightWidth, pathHighlightWidth;
        float highlightAlpha, pathHighlightAlpha, nodeConnectionAlpha;
        /* num random obstacles
        */
        int numObstacles;
        
        /* util functions
        */
        int getIdx(int i, int j);
        bool isCellFree(int i, int j);
        bool isCellObstacle(int i, int j);
        bool isCellEndCell(int i, int j);
        void setCellColorFromState(int i, int j, cellState state, float alpha = 1.0);
        void setCellBlockToState(int i, int j, cellState state, int width);
        void setCellAsFree(int i, int j);
        void setCellBlockAsFree(int i, int j, int width);
        void setCellAsObstacle(int i, int j);
        void setCellAsNode(int i, int j);
        void setCellAsNodeConnection(int i, int j);
        void setCellAsStartCell(int i, int j);
        void setCellAsEndCell(int i, int j);
        void setCellAsObstacleStream(int i1, int j1, int i2, int j2, const int width, 
        widthType wType);
        void setCellAsNodeConnectionStream(int i1, int j1, int i2, int j2);
        void clearNodeConnectionStream(int i1, int j1, int i2, int j2);
        std::vector<std::pair<int, int> > connectTwoCells(int i1, int j1, int i2, int j2);
        void highlightCell(int i, int j, cellState state);
        void highlightPath(std::vector<std::pair<int, int>> path, cellState state);
        void deHighlightCell(int i, int j);
        void deHighlightPath(std::vector<std::pair<int, int>> path);
        void restartRenderLoop(void);
        int getRandomAmount(int start, int end);
        float getDistanceBetweenCells(int i1, int j1, int i2, int j2);
        void setRandomObstacles(int numObstacles);

        /* primary functions
        */
        std::pair<int, int> getRandomCell(void);
        std::pair<int, int> getNearestNode(std::pair<int, int> rNode);
        bool isNodeValid(std::pair<int, int> nearestNode, std::pair<int, int>& newNode);
        bool computeNewNodeAndValidate(std::pair<int, int> rNode, std::pair<int, int>& newNode);
        bool createAndConnectNewNode(std::pair<int, int> nearestNode, 
        std::pair<int, int> newNode);
        bool placeNodeRRT(std::pair<int, int> rNode, std::pair<int, int>& newNode);
        bool placeNodeRRTStar(std::pair<int, int> rNode, std::pair<int, int>& newNode);
        bool isGoalReached(std::pair<int, int> dNode);
        bool isPathAlreadyExist(std::pair<int, int>& lastNode);

    public:
        RandomTreeClass(int _step, int _neighborhood, int _N, int _scale, bool noStroke);
        ~RandomTreeClass(void);

        /* override functions
        */
        void setObstacleCells(void);
        void setStartAndEndCells(void);
        void simulationStep(void);
};
#endif /* SIMULATION_RANDOMTREE_H
*/
