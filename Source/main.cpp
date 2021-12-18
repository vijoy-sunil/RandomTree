#include "../Include/Simulation/Constants.h"
#include "../Include/Simulation/RandomTree.h"

int main(void){
    RandomTreeClass RandomTree(step, neighborhood, N, scale, true);
    RandomTree.runRender();
    return 0;
}