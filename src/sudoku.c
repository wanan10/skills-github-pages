#include "sudoku.h"
#include <string.h>

static void at_least_one_digit(CNF *cnf, int r, int c) {
	cnf_start_clause(cnf);
	for (int d = 1; d <= 9; d++) cnf_add_lit(cnf, v(r,c,d));
	cnf_add_lit(cnf, 0);
	cnf_end_clause(cnf);
}

static void at_most_one_digit(CNF *cnf, int r, int c) {
	for (int d1 = 1; d1 <= 9; d1++) {
		for (int d2 = d1 + 1; d2 <= 9; d2++) {
			cnf_start_clause(cnf);
			cnf_add_lit(cnf, -v(r,c,d1));
			cnf_add_lit(cnf, -v(r,c,d2));
			cnf_add_lit(cnf, 0);
			cnf_end_clause(cnf);
		}
	}
}

static void row_constraints(CNF *cnf) {
	for (int r = 1; r <= 9; r++) {
		for (int d = 1; d <= 9; d++) {
			// at least one in row r for digit d
			cnf_start_clause(cnf);
			for (int c = 1; c <= 9; c++) cnf_add_lit(cnf, v(r,c,d));
			cnf_add_lit(cnf, 0);
			cnf_end_clause(cnf);
			// at most one in row r for digit d
			for (int c1 = 1; c1 <= 9; c1++) for (int c2 = c1+1; c2 <= 9; c2++) {
				cnf_start_clause(cnf);
				cnf_add_lit(cnf, -v(r,c1,d));
				cnf_add_lit(cnf, -v(r,c2,d));
				cnf_add_lit(cnf, 0);
				cnf_end_clause(cnf);
			}
		}
	}
}

static void col_constraints(CNF *cnf) {
	for (int c = 1; c <= 9; c++) {
		for (int d = 1; d <= 9; d++) {
			cnf_start_clause(cnf);
			for (int r = 1; r <= 9; r++) cnf_add_lit(cnf, v(r,c,d));
			cnf_add_lit(cnf, 0);
			cnf_end_clause(cnf);
			for (int r1 = 1; r1 <= 9; r1++) for (int r2 = r1+1; r2 <= 9; r2++) {
				cnf_start_clause(cnf);
				cnf_add_lit(cnf, -v(r1,c,d));
				cnf_add_lit(cnf, -v(r2,c,d));
				cnf_add_lit(cnf, 0);
				cnf_end_clause(cnf);
			}
		}
	}
}

static void box_constraints(CNF *cnf) {
	for (int br = 0; br < 3; br++) for (int bc = 0; bc < 3; bc++) {
		for (int d = 1; d <= 9; d++) {
			cnf_start_clause(cnf);
			for (int dr = 1; dr <= 3; dr++) for (int dc = 1; dc <= 3; dc++)
				cnf_add_lit(cnf, v(br*3+dr, bc*3+dc, d));
			cnf_add_lit(cnf, 0);
			cnf_end_clause(cnf);
			// at most one per box for digit d
			for (int r1 = 1; r1 <= 3; r1++) for (int c1 = 1; c1 <= 3; c1++)
			for (int r2 = r1; r2 <= 3; r2++) for (int c2 = (r2==r1?c1+1:1); c2 <= 3; c2++) {
				cnf_start_clause(cnf);
				cnf_add_lit(cnf, -v(br*3+r1, bc*3+c1, d));
				cnf_add_lit(cnf, -v(br*3+r2, bc*3+c2, d));
				cnf_add_lit(cnf, 0);
				cnf_end_clause(cnf);
			}
		}
	}
}

void sudoku_add_base_rules(CNF *cnf) {
	// each cell exactly one digit
	for (int r = 1; r <= 9; r++) for (int c = 1; c <= 9; c++) {
		at_least_one_digit(cnf, r, c);
		at_most_one_digit(cnf, r, c);
	}
	row_constraints(cnf);
	col_constraints(cnf);
	box_constraints(cnf);
	if (cnf->num_vars < 9*9*9) cnf->num_vars = 9*9*9;
}

int sudoku_add_clues(CNF *cnf, const char *grid81) {
	if (!grid81 || (int)strlen(grid81) != 81) return -1;
	for (int r = 1; r <= 9; r++) for (int c = 1; c <= 9; c++) {
		char ch = grid81[(r-1)*9 + (c-1)];
		if (ch == '.' || ch == '0') continue;
		if (ch < '1' || ch > '9') return -1;
		int d = ch - '0';
		cnf_start_clause(cnf);
		cnf_add_lit(cnf, v(r,c,d));
		cnf_add_lit(cnf, 0);
		cnf_end_clause(cnf);
	}
	return 0;
}

