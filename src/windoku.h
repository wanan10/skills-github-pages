#ifndef WINDOKU_H
#define WINDOKU_H

#include "cnf.h"
#include "sudoku.h"

// Add percent (撇对角线) constraints: two main diagonals have all digits distinct
void windoku_add_diagonals(CNF *cnf);

// Add window constraints: four 3x3 windows centered at (2,2), (2,6), (6,2), (6,6)
// Each window requires digits 1..9 exactly once (like sub-boxes but shifted)
void windoku_add_windows(CNF *cnf);

#endif

