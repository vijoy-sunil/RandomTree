#ifndef UTILS_TREE_H
#define UTILS_TREE_H

#include <vector>
#include <map>

typedef struct node{
    std::pair<int, int> pos;
    struct node *parent;
    int distanceFromParent;
    std::vector<node*> children;
}node_t;

class TreeClass{
    private:
        node_t *root;
        std::map<std::pair<int, int>, node_t*> mp;

        bool getNumChildren(node_t* source);
        node_t* getNodeFromCell(int i, int j);

    public:
        TreeClass(void);
        ~TreeClass(void);

        bool createNode(std::pair<int, int> cellPos);
        bool addEdge(node_t *source, node_t *dest, int dist);
        bool removeEdge(node_t *source, node_t *dest);
};
#endif /* UTILS_TREE_H
*/
