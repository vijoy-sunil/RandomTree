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
    /* used for final path highlight alpha
    */
    pathHighlightAlpha = 0.5;
    /* used for node connection alpha
    */
    nodeConnectionAlpha = 0.2;
    /* num random obstacles
    */
    numObstacles = 0.02 * N;
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

/* get nearest node coords from random node, calculate distance to
 * all nodes using the map
*/
std::pair<int, int> RandomTreeClass::getNearestNode(std::pair<int, int> rNode){
    float minDistance = INT_MAX;
    /* nearest node coords
    */
    int minX, minY;
    /* iterate over all nodes
    */
    for(auto it = mp.begin(); it != mp.end(); it++){
        int nodeX = it->first.first;
        int nodeY = it->first.second;

        float currDistance = getDistanceBetweenCells(rNode.first, rNode.second, nodeX, nodeY);
        if(currDistance < minDistance){
            minDistance = currDistance;
            minX = nodeX;
            minY = nodeY;
        }
    }
    return std::make_pair(minX, minY);
}

/* The generated node has to be in the free space, and the path 
 * connecting it and the nearest node shouldn't pass through any 
 * obstacle
*/
bool RandomTreeClass::isNodeValid(std::pair<int, int> nearestNode, std::pair<int, int>& newNode){
    int nearX = nearestNode.first;
    int nearY = nearestNode.second;

    int newX = newNode.first;
    int newY = newNode.second;

    /* the connectTwoCells outputs the input (i,j) cell
     * as well, so no need to test it separately, but we
     * do need to skip the check for the nearest node
    */
    std::vector<std::pair<int, int> > points;
    points = connectTwoCells(nearX, nearY, newX, newY);

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
            pathFound = true;
            newNode = std::make_pair(px, py);
            std::cout<<"Path Found while validation"<<std::endl;
            return true;
        }
    }
    return true;
}

bool RandomTreeClass::computeNewNodeAndValidate(std::pair<int, int> rNode, 
std::pair<int, int>& newNode){
    std::pair<int, int> nearestNode = getNearestNode(rNode);
    int nearX = nearestNode.first;
    int nearY = nearestNode.second;

    float d = getDistanceBetweenCells(rNode.first, rNode.second, nearX, nearY);
    int newNodeX, newNodeY;
    /* https://math.stackexchange.com/questions/175896/finding
     * -a-point-along-a-line-a-certain-distance-away-from-another-point
    */
    float t;
    /* If the random node is too close (<step) to the nearest
     * node, we choose the random node as the new node
    */
    if(d <= step){
        newNodeX = rNode.first;
        newNodeY = rNode.second;
    }
    else{
        t = step/d;
        newNodeX = ((1 - t) * nearX) + (t * rNode.first);
        newNodeY = ((1 - t) * nearY) + (t * rNode.second);
    }

    /* NOTE: there is a chance that we find the end cell block
     * while validation, in that case we will overwrite newNodeX,Y
    */
    newNode = std::make_pair(newNodeX, newNodeY);
    if(!isNodeValid(nearestNode, newNode)){
        std::cout<<"Retrying . . . [Invalid new node]"<<std::endl;
        return false;
    }    
    return true;
}

bool RandomTreeClass::createAndConnectNewNode(std::pair<int, int> nearestNode, 
std::pair<int, int> newNode){
    /* add node to tree and connect
    */ 
    if(createNode(newNode)){
        /* add edge
         * NOTE: source node will always be the parent node
        */
        node_t *source = getNodeFromCell(nearestNode.first, nearestNode.second);
        node_t *dest = getNodeFromCell(newNode.first, newNode.second);

        if(!addEdge(source, dest))
            assert(false);

        /* check if you have reached goal after the addition of
         * new node
        */
        if(isGoalReached(newNode))
            pathFound = true;
        /* set cell state
        */
        setCellAsNode(newNode.first, newNode.second);
        /* add connection path
        */
        setCellAsNodeConnectionStream(nearestNode.first, nearestNode.second, 
        newNode.first, newNode.second);
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

/* place node 1 step away from nearest node along the line
 * connecting nearest node and random node at (i,j). Validate
 * the new node before placing it. Finally return nearest node 
 * to draw the path as part of algorithm
*/
bool RandomTreeClass::placeNodeRRT(std::pair<int, int> rNode, std::pair<int, int>& newNode){
    if(!computeNewNodeAndValidate(rNode, newNode))
        return false;

    /* save the nearest node
    */
    std::pair<int, int> nearestNode = getNearestNode(rNode);
    int nearX = nearestNode.first;
    int nearY = nearestNode.second;
    std::cout<<"Nearest Node: "<<nearX<<","<<nearY<<std::endl;
    std::cout<<"New Node placed: "<<newNode.first<<","<<newNode.second<<std::endl;

    return createAndConnectNewNode(nearestNode, newNode);
}

/* node placement algorithm for RRT*
*/
bool RandomTreeClass::placeNodeRRTStar(std::pair<int, int> rNode, std::pair<int, int>& newNode){
    /* this step is same as RRT, find newNode that is step away 
     * from the nearest node
    */
    if(!computeNewNodeAndValidate(rNode, newNode))
        return false;    

    std::cout<<"New Node: "<<newNode.first<<","<<newNode.second<<std::endl;

    float minCost = INT_MAX;
    node_t *minCostNeighborNode;
    std::vector<node_t*> neighborhoodNodes;

    /* find nodes that are within the neighborhood distance of 
     * newNode and compute minimum cost path to newNode
    */
    for(auto it = mp.begin(); it != mp.end(); it++){
        std::pair<int, int> currNodePos = it->first;
        node_t* currNode = it->second;

        float d = getDistanceBetweenCells(newNode.first, newNode.second, 
        currNodePos.first, currNodePos.second);
        if(d <= neighborhood){
            /* Find minimum cost path to reach newNode through the neighb-
             * -orhood nodes. Before that we need to validate this neighbor
             * hood node connection to newNode
            */ 
            if(!isNodeValid(currNodePos, newNode)){
                std::cout<<"Retrying . . . [Invalid new node]"<<std::endl;
                continue;
            }  
            /* if path has been found with newNode (may or may not have 
             * been overwritten), then we need to break out of this loop
             * and form edge
            */
            if(pathFound){
                /* reach here if path has been found while validation of connection
                 * between currNode and newNode
                */
                if(!createAndConnectNewNode(currNodePos, newNode))
                    return false;   
                else
                    return true;
            }

            /* save valid neighborhood nodes
            */
            neighborhoodNodes.push_back(currNode);
            /* compute cost to newNode through currNode
            */
            d = getDistanceToRoot(currNode) + 
            getDistanceBetweenCells(newNode.first, newNode.second, 
            currNodePos.first, currNodePos.second);

            std::cout<<"Neighborhood Node: "<<currNodePos.first<<","<<currNodePos.second<<" ";
            std::cout<<"Cost To New Node: "<<d<<std::endl;

            /* save min cost node
            */
            if(d < minCost){
                minCost = d;
                minCostNeighborNode = currNode;
            }

        }
    }

    std::cout<<"Min Cost Neighbor Node: "<<minCostNeighborNode->pos.first<<","
    <<minCostNeighborNode->pos.second<<std::endl;

    /* add edge from min cost neighbor node to new node
    */
    if(!createAndConnectNewNode(minCostNeighborNode->pos, newNode))
        return false;
    /* Next, go through the neighborhood nodes and check if we
     * can reduce the cost from root to itself by going through
     * the newNode
    */
   
    node_t* currNode = getNodeFromCell(newNode.first, newNode.second);
    for(int k = 0; k < neighborhoodNodes.size(); k++){
        float dToNNode = getDistanceToRoot(neighborhoodNodes[k]);

        std::cout<<"Neighborhood Node: "<<neighborhoodNodes[k]->pos.first<<","
        <<neighborhoodNodes[k]->pos.second<<" ";
        std::cout<<"dToNNode: "<<dToNNode<<std::endl;

        float dNewBridge = getDistanceBetweenCells(newNode.first, newNode.second, 
        neighborhoodNodes[k]->pos.first, neighborhoodNodes[k]->pos.second);
        std::cout<<"dNewBridge: "<<dNewBridge<<std::endl;

        float dNewRoute = getDistanceToRoot(currNode) + dNewBridge;
        std::cout<<"dNewRoute: "<<dNewRoute<<std::endl;

        if(dNewRoute < dToNNode){
            std::cout<<"Rerouting . . ."<<std::endl;
            if(!removeEdge(neighborhoodNodes[k]->parent, neighborhoodNodes[k]))
                assert(false);
            if(!addEdge(currNode, neighborhoodNodes[k]))
                assert(false);
        }
    }
    
    return true;
}

/* check if one of the 8 neighbors is an end cell. 
 * This is when we move the end cell block after a path
 * has already been found
 * 
 * In case of normal operation, check if the cell itself
 * is an end cell
*/
bool RandomTreeClass::isGoalReached(std::pair<int, int> dNode){
    int i = dNode.first;
    int j = dNode.second;
    
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
    /* check if any of the added nodes are an end cell coord
    */
    for(auto it = mp.begin(); it != mp.end(); it++){
        if(isGoalReached(it->first)){
            lastNode = it->first;
            return true;
        }
    }
    return false;
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
    setRandomObstacles(numObstacles);
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
#if STEP_MODE == 1
    if(stepMode){
        /* add delay, this to enable single step mode
        */
        int delayCnt = 90000000;
        while(delayCnt--);
#endif
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
                placeNodeRRT(rNode, newNode); 
#endif
#if RAPID_RANDOM_TREE_STAR == 1
                placeNodeRRTStar(rNode, newNode);
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
#if STEP_MODE == 1
        /* set to true in process input fn
        */ 
        stepMode = false;
    }
#endif
}