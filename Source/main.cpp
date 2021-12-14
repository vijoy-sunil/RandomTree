#include "../Include/Simulation/Constants.h"
#include "../Include/Simulation/RandomTree.h"

int main(void){
    RandomTreeClass RandomTree(step, N, scale, true);
    RandomTree.runRender();
    return 0;
}