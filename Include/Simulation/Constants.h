#ifndef SIMULATION_CONSTANTS_H
#define SIMULATION_CONSTANTS_H

/* step through render loop
*/
#define STEP_MODE               0
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
const int neighborhood = 5.0 * step;
/* algorithm options
*/
#define RAPID_RANDOM_TREE         0
#define RAPID_RANDOM_TREE_STAR    1
#endif /* SIMULATION_CONSTANTS_H
*/
