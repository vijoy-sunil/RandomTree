#ifndef SIMULATION_CONSTANTS_H
#define SIMULATION_CONSTANTS_H

/* step through render loop
*/
#define STEP_MODE               1
/* grid dimension NxN
*/
const int N = 800;
/* screen scale factor
*/
const int scale = 1;
/* choose obstacle pattern
*/
#define PREDEF_OBSTACLE         1
#define RANDOM_OBSTACLE         0
/* Algorithm params
*/
const int step = 10;
const int neighborhood = 20;
/* algorithm options
*/
#define RAPID_RANDOM_TREE         1
#define RAPID_RANDOM_TREE_STAR    0
#endif /* SIMULATION_CONSTANTS_H
*/
