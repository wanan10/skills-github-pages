#ifndef SUDOKU_H
#define SUDOKU_H

#include "cnf.h"

// Convert a 9x9 Sudoku grid (0 for empty) to CNF
CNF *sudoku_to_cnf(int grid[9][9]);

#endif // SUDOKU_H
