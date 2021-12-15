#include "../../Include/Utils/Tree.h"
#include <stdlib.h>

TreeClass::TreeClass(void){
    root = NULL;
}

TreeClass::~TreeClass(void){

}

bool TreeClass::getNumChildren(node_t *source){
    if(source == NULL)
        return 0;

    return source->children.size();
}

node_t* TreeClass::getNodeFromCell(int i, int j){
    std::pair<int, int> cellPos = std::make_pair(i, j);

    if(mp.find(cellPos) != mp.end())
        return mp[std::make_pair(i, j)];
    else
        return NULL;
}

bool TreeClass::createNode(std::pair<int, int> cellPos){
    node_t *newNode = (node_t*)malloc(sizeof(node_t));
    newNode->pos = cellPos;
    newNode->parent = NULL;
    newNode->distanceFromParent = 0;

    if(root == NULL)
        root = newNode;

    /* update map to help in retreiving the node using cell
     * coordinates
    */
    if(mp.find(cellPos) == mp.end()){
        mp[cellPos] = newNode;
        return true;
    }
    else
        return false;
}

/* NOTE: source will be the parent and dest will be a child
 * always
*/
bool TreeClass::addEdge(node_t *source, node_t *dest, int dist){
    if(source == NULL || dest == NULL)
        return false;

    /* from parent pov
    */
    source->children.push_back(dest);
    /* from child's pov
    */
    dest->parent = source;
    dest->distanceFromParent = dist;
    return true;
}

/* this call will be followed by reconnection with another node
 * since we have found a lower cost path
*/
bool TreeClass::removeEdge(node_t *source, node_t *dest){
    if(source == NULL || dest == NULL)
        return false;

    /* source is the parent of dest, so first we need to
     * remove that connection
    */
    source->children.erase(std::remove(source->children.begin(), source->children.end(), 
                           dest), source->children.end());
    /* we are not clearing any dest values since they will
     * be reconnected next time and overwritten
    */
    return true;
}

