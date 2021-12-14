#include "../../Include/Utils/Common.h"

/* Global variables common
*/
/* This is set to true when the start and end goal cells
 * are set
*/
volatile bool readyToStart = false;
/* this is set when the start cell is set, similar op
 * with endCellSet
*/
volatile bool startCellSet = false;
volatile bool endCellSet = false;
/* mouse click position
*/
double xPos, yPos;
/* this is needed so that we operate only once in the loop.
 * Also, we compute cellX, cellY only if this boolean is 
 * set
*/
volatile bool mouseClicked = false;
/* call back function that is registered to be called upon
 * mouse click
*/