#ifndef UTILS_TREE_H
#define UTILS_TREE_H

#include <vector>
#include <map>

typedef struct node{
    /* node coordinates
    */
    std::pair<int, int> pos;
    /* float distance to root
    */
    float distanceToRoot;
    /* parent of the node
    */
    struct node *parent;
}node_t;

class TreeClass{
    private:
        /* the root node will be the start cell
        */
        node_t *root;

    protected:
        /* the map keeps account of all the nodes
        */
        std::map<std::pair<int, int>, node_t*> mp;

        void showMap(void);
        node_t* getNodeFromCell(int i, int j);
        std::vector<std::pair<int, int>> getPath(std::pair<int, int> lastAddedNode);

    public:
        TreeClass(void);
        ~TreeClass(void);

        bool createNode(std::pair<int, int> cellPos);
        bool addEdge(node_t *source, node_t *dest, float distance);
};
#endif /* UTILS_TREE_H
*/
