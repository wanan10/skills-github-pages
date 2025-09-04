#ifndef SUDOKU_H
#define SUDOKU_H

#include "cnf.h"

// Map (row, col, digit) -> variable id in [1..729]
static inline int v(int row, int col, int digit) {
	return (row - 1) * 81 + (col - 1) * 9 + digit;
}

void sudoku_add_base_rules(CNF *cnf);
// Add clues from a 9x9 grid string (81 chars, '0' or '.' for empty)
int sudoku_add_clues(CNF *cnf, const char *grid81);

#endif

