#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnf.h"

// Simple 9x9 Sudoku encoding (classic). %-Sudoku specifics can be added later.

static int var_id(int r, int c, int d) { // 1-based r,c,d in 1..9
	return (r - 1) * 81 + (c - 1) * 9 + d;
}

CNF *sudoku_to_cnf(int grid[9][9]) {
	CNFBuilder b;
	cnf_builder_init(&b, 9 * 9 * 9, 20000, 200000);

	int lits[9];
	// Cell constraints: each cell has at least one number
	for (int r = 1; r <= 9; ++r) {
		for (int c = 1; c <= 9; ++c) {
			for (int d = 1; d <= 9; ++d) lits[d - 1] = var_id(r, c, d);
			cnf_builder_add_clause(&b, lits, 9);
			// At most one number per cell
			for (int d1 = 1; d1 <= 9; ++d1) for (int d2 = d1 + 1; d2 <= 9; ++d2) {
				int pair[2] = { -var_id(r, c, d1), -var_id(r, c, d2) };
				cnf_builder_add_clause(&b, pair, 2);
			}
		}
	}
	// Row constraints
	for (int r = 1; r <= 9; ++r) {
		for (int d = 1; d <= 9; ++d) {
			for (int c = 1; c <= 9; ++c) lits[c - 1] = var_id(r, c, d);
			cnf_builder_add_clause(&b, lits, 9);
			for (int c1 = 1; c1 <= 9; ++c1) for (int c2 = c1 + 1; c2 <= 9; ++c2) {
				int pair[2] = { -var_id(r, c1, d), -var_id(r, c2, d) };
				cnf_builder_add_clause(&b, pair, 2);
			}
		}
	}
	// Column constraints
	for (int c = 1; c <= 9; ++c) {
		for (int d = 1; d <= 9; ++d) {
			for (int r = 1; r <= 9; ++r) lits[r - 1] = var_id(r, c, d);
			cnf_builder_add_clause(&b, lits, 9);
			for (int r1 = 1; r1 <= 9; ++r1) for (int r2 = r1 + 1; r2 <= 9; ++r2) {
				int pair[2] = { -var_id(r1, c, d), -var_id(r2, c, d) };
				cnf_builder_add_clause(&b, pair, 2);
			}
		}
	}
	// Box constraints
	for (int br = 0; br < 3; ++br) for (int bc = 0; bc < 3; ++bc) {
		for (int d = 1; d <= 9; ++d) {
			int idx = 0;
			for (int r = 1; r <= 3; ++r) for (int c = 1; c <= 3; ++c) {
				lits[idx++] = var_id(br * 3 + r, bc * 3 + c, d);
			}
			cnf_builder_add_clause(&b, lits, 9);
			for (int i = 0; i < 9; ++i) for (int j = i + 1; j < 9; ++j) {
				int pair[2] = { -lits[i], -lits[j] };
				cnf_builder_add_clause(&b, pair, 2);
			}
		}
	}
	// Clues
	for (int r = 1; r <= 9; ++r) for (int c = 1; c <= 9; ++c) if (grid[r - 1][c - 1] >= 1 && grid[r - 1][c - 1] <= 9) {
		int lit = var_id(r, c, grid[r - 1][c - 1]);
		cnf_builder_add_clause(&b, &lit, 1);
	}

	CNF *cnf = cnf_builder_finalize(&b);
	cnf_builder_free(&b);
	return cnf;
}

