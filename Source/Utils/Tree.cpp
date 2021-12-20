#include "../../Include/Utils/Tree.h"
#include <stdlib.h>
#include <iostream>
#include <cmath>

TreeClass::TreeClass(void){
    root = NULL;
}

TreeClass::~TreeClass(void){
    /* free all nodes
    */
    for(auto it = mp.begin(); it != mp.end(); it++)
        free(it->second);
}

/* (i,j) -|- mode_t* are the map contents
*/
void TreeClass::showMap(void){
    for(auto it = mp.begin(); it != mp.end(); it++){
        std::cout<<"("<<it->first.first<<","<<it->first.second<<")";
        std::cout<<"\t"<<it->second<<std::endl;
    }
}

node_t* TreeClass::getNodeFromCell(int i, int j){
    std::pair<int, int> cellPos = std::make_pair(i, j);

    if(mp.find(cellPos) != mp.end())
        return mp[cellPos];
    else
        return NULL;
}

/* get all node positions starting from lastAddedNode to start cell
*/
std::vector<std::pair<int, int>> TreeClass::getPath(std::pair<int, int> lastAddedNode){
    std::vector<std::pair<int, int>> solvedPath;
    node_t *currNode = getNodeFromCell(lastAddedNode.first, lastAddedNode.second);
    if(currNode == NULL)
        assert(false);
        
    solvedPath.push_back(currNode->pos);
    while(currNode->parent != NULL){
        currNode = currNode->parent;
        solvedPath.push_back(currNode->pos);
    }
    return solvedPath;
}

bool TreeClass::createNode(std::pair<int, int> cellPos){
    node_t *newNode = (node_t*)malloc(sizeof(node_t));
    newNode->pos = cellPos;
    newNode->parent = NULL;

    if(root == NULL){
        root = newNode;
    }

    /* update map to help in retreiving the node using cell
     * coordinates or freeing up node memory
    */
    if(mp.find(cellPos) == mp.end()){
        mp[cellPos] = newNode;
        return true;
    }
    else{
        free(newNode);
        return false;
    }
}

/* NOTE: source will be the parent and dest will be a child
 * always
*/
bool TreeClass::addEdge(node_t* source, node_t* dest){
    if(source == NULL || dest == NULL)
        return false;

    /* from child's pov
    */
    dest->parent = source;
    return true;
}

bool TreeClass::removeEdge(node_t* source, node_t* dest){
    if(source == NULL || dest == NULL)
        return false;

    /* from child's pov
    */   
    dest->parent = NULL;
    return true;
}

float TreeClass::getDistanceToRoot(node_t* dest){
    float d = 0;
    if(dest == NULL)
        return d;

    while(dest->parent != NULL){
        int i1 = dest->parent->pos.first;
        int j1 = dest->parent->pos.second;

        int i2 = dest->pos.first;
        int j2 = dest->pos.second;
        d += sqrt(pow((j2 - j1), 2) + pow((i2 - i1), 2));
        
        dest = dest->parent;
    }
    return d;
}


