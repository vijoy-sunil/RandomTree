#include "../../Include/Simulation/RandomTree.h"
#include "../../Include/Simulation/Constants.h"
#include "../../Include/Utils/Common.h"
#include <iostream>

RandomTreeClass::RandomTreeClass(int _step, int _neighborhood, int _N, int _scale, bool noStroke): 
GridClass(_N, _scale, noStroke), TreeClass(){
    cellCurr = (int*)calloc(N * N, sizeof(int));
    step = _step;
    neighborhood = _neighborhood;
    /* will be alive throughout the life of the program
    */
    numNodesAdded = 0;

    startX = 0; startY = 0;
    endX = 0; endY = 0;

    endCellWidth = 0.02 * N;
    borderWidth = 0.03 * N;

    pathFound = false;
    /* this highlight width is used for START_CELL or NODE
    */
    otherCellHighlightWidth = 0.01 * N;
    /* this highlight width is used for END_CELL
    */
    endCellHighlightWidth = 0.03 * N;
    /* final path highlight width
    */
    pathHighlightWidth = 2;
    /* used for START_CELL or END_CELL highlight alpha, for other
     * states it will be 1.0
    */
    highlightAlpha = 0.3;
}

RandomTreeClass::~RandomTreeClass(void){
    free(cellCurr);
}

/* first step in path generation, a random node in free space
 * is generated
*/
std::pair<int, int> RandomTreeClass::getRandomCell(void){
    std::pair<int, int> randomCell;
    int randomX, randomY;
    /* retry if the random cell is not valid
    */
    while(1){
        randomX = getRandomAmount(0, N-1);
        randomY = getRandomAmount(0, N-1);
        /* validate node
        */
        if(!isCellObstacle(randomX, randomY)){
            randomCell = std::make_pair(randomX, randomY);
            break;
        }
    }
    std::cout<<"Random Node "<<randomX<<","<<randomY<<std::endl;
    return randomCell;
}

/* get nearest node coords from (i,j), calculate distance to
 * all nodes using the map
*/
std::pair<int, int> RandomTreeClass::getNearestNode(int i, int j){
    float minDistance = INT_MAX;
    /* nearest node coords
    */
    int minX, minY;
    /* iterate over all nodes
    */
    for(auto it = mp.begin(); it != mp.end(); it++){
        int nodeX = it->first.first;
        int nodeY = it->first.second;

        float currDistance = getDistanceBetweenCells(i, j, nodeX, nodeY);
        if(currDistance < minDistance){
            minDistance = currDistance;
            minX = nodeX;
            minY = nodeY;
        }
    }
    return std::make_pair(minX, minY);
}

float RandomTreeClass::getDistanceBetweenCells(int i1, int j1, int i2, int j2){
    return sqrt(pow((j2 - j1), 2) + pow((i2 - i1), 2));
}

/* The generated node has to be in the free space, and the path 
 * connecting it and the nearest node shouldn't pass through any 
 * obstacle
*/
bool RandomTreeClass::isNodeValid(int& i, int& j){
    std::pair<int, int> nearestNode = getNearestNode(i, j);
    int nearX = nearestNode.first;
    int nearY = nearestNode.second;

    /* the connectTwoCells outputs the input (i,j) cell
     * as well, so no need to test it separately, but we
     * do need to skip the check for the nearest node
    */
    std::vector<std::pair<int, int> > points;
    points = connectTwoCells(nearX, nearY, i, j);

    int px, py;
    for(int k = 0; k < points.size(); k++){ 
        px = points[k].first;
        py = points[k].second;
        /* skip the check for the nearest node
        */
        if(px == nearX && py == nearY)
            continue;

        if(isCellObstacle(px, py))
            return false;  

        /* if the line from nearest node and generated node passes
         * through the end cell block
        */ 
        if(isCellEndCell(px, py)){
            /* rewrite the generate node
            */
            i = px;
            j = py;
            std::cout<<"Path Found while validation"<<std::endl;
            return true;
        }
    }
    return true;
}

/* place node 1 step away from nearest node along the line
 * connecting nearest node and random node at (i,j). Validate
 * the new node before placing it. Finally return nearest node 
 * to draw the path as part of algorithm
*/
bool RandomTreeClass::placeNode(int i, int j, std::pair<int, int>& newNode){
    std::pair<int, int> nearestNode = getNearestNode(i, j);
    int nearX = nearestNode.first;
    int nearY = nearestNode.second;

    float d = getDistanceBetweenCells(nearX, nearY, i, j);
    int newNodeX, newNodeY;
    /* https://math.stackexchange.com/questions/175896/finding
     * -a-point-along-a-line-a-certain-distance-away-from-another-point
    */
    float t;
    /* If the random node is too close (<step) to the nearest
     * node, we choose the random node as the new node
    */
    if(d <= step){
        newNodeX = i;
        newNodeY = j;
    }
    else{
        t = step/d;
        newNodeX = ((1 - t) * nearX) + (t * i);
        newNodeY = ((1 - t) * nearY) + (t * j);
    }

    /* NOTE: there is a chance that we find the end cell block
     * while validation, in that case we will overwrite newNodeX,Y
    */
    if(!isNodeValid(newNodeX, newNodeY)){
        std::cout<<"Retrying . . . [Invalid new node]"<<std::endl;
        return false;
    }  

    /* add node to tree
    */ 
    if(createNode(std::make_pair(newNodeX, newNodeY))){
        std::cout<<"Nearest Node: "<<nearX<<","<<nearY<<std::endl;
        std::cout<<"New Node placed at: "<<newNodeX<<","<<newNodeY<<std::endl;
        /* add edge
         * NOTE: source node will always be the parent node
        */
        node_t *source = getNodeFromCell(nearX, nearY);
        node_t *dest = getNodeFromCell(newNodeX, newNodeY);
        if(!addEdge(source, dest))
            assert(false);

        /* check if you have reached goal after the addition of
         * new node
        */
        if(isGoalReached(newNodeX, newNodeY))
            pathFound = true;
        /* set cell state
        */
        setCellAsNode(newNodeX, newNodeY);
        /* add connection path
        */
        setCellAsNodeConnectionStream(nearX, nearY, newNodeX, newNodeY);
        /* store result
        */
        newNode = std::make_pair(newNodeX, newNodeY);
        numNodesAdded++;
        return true;
    }
    /* if you are here, then createNode() failed, so we need to
     * retry again
    */
   else{
        std::cout<<"Retrying . . . [New node already exists]"<<std::endl;
        return false;
   }
}

/* check if one of the 8 neighbors is an end cell. 
 * This is when we move the end cell block after a path
 * has already been found
 * 
 * In case of normal operation, check if the cell itself
 * is an end cell
*/
bool RandomTreeClass::isGoalReached(int i, int j){
    for(int r = -1; r <= 1; r++){
        for(int c = -1; c <= 1; c++){
            /* boundary guards
            */
            if(i + r < 0 || i + r > N-1)
                continue;
            if(j + c < 0 || j + c > N-1)
                continue;
                
            if(isCellEndCell(i + r, j + c))
                return true;
        }
    }
    return false;
}

/* check if a path already exists before starting the algorithm
*/
bool RandomTreeClass::isPathAlreadyExist(std::pair<int, int>& lastNode){
    int px, py;
    /* check if any of the added nodes are an end cell coord
    */
    for(auto it = mp.begin(); it != mp.end(); it++){
        px = it->first.first;
        py = it->first.second;

        if(isGoalReached(px, py)){
            lastNode = std::make_pair(px, py);
            return true;
        }
    }
    return false;
}

/* check if (i2,j2) is within step distance of (i1,j1)
*/
bool RandomTreeClass::isWithinStepDistance(int i1, int j1, int i2, int j2){
    float distance = getDistanceBetweenCells(i1, j1, i2, j2);
    return distance <= step;
}

/* check if (i2,j2) is within neighborood distance of (i1,j1)
*/
bool RandomTreeClass::isWithinNeighborhoodDistance(int i1, int j1, int i2, int j2){
    float distance = getDistanceBetweenCells(i1, j1, i2, j2);
    return distance <= neighborhood;
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
    setCellAsObstacleStream(N/1.5, 0.8 * N, N-1, 0.8 * N, borderWidth, BOTTOM); 
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
        /* Now set the end goal cell
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
            /* add start cell to tree
            */
            if(createNode(std::make_pair(startX, startY)))
                std::cout<<"Added START CELL to tree"<<std::endl;
            else
                std::cout<<"START CELL already exists in map"<<std::endl;
        }
    }
}

void RandomTreeClass::simulationStep(void){
    /* this is set to false via reset or if start and end cells have
     * not been selected
    */
    if(readyToStart){
        /* holds last added node that reaced the end cell
        */
        std::pair<int, int> newNode;
        /* STEP 0, check if a path already exists
        */
        if(isPathAlreadyExist(newNode))
            pathFound = true;
        else{
            /* STEP1, get a valid random node
            */
            std::pair<int, int> rNode = getRandomCell();
            /* STEP2, place node at step away from nearest node
            */
#if RAPID_RANDOM_TREE == 1
            placeNode(rNode.first, rNode.second, newNode); 
#endif

#if RAPID_RANDOM_TREE_STAR == 1
#endif
        }
        /* STEP 3, check if you have reached end cell
        */
        if(pathFound){
            /* clear previous path before computing a new one
            */
            if(path.size() != 0){
                deHighlightPath(path);
                path.clear();
            }
            
            std::cout<<"Goal Reached !!! "<<newNode.first<<","<<newNode.second<<std::endl;
            std::cout<<"Number of Nodes Added: "<<numNodesAdded<<std::endl;
            path = getPath(newNode);
            /* display path
            */
            highlightPath(path, END_CELL);
            restartRenderLoop();
        }   
    }
}