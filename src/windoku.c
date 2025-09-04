#include "windoku.h"

static void pairwise_all_diff(CNF *cnf, int vars[9]) {
	// at least one
	cnf_start_clause(cnf);
	for (int i = 0; i < 9; i++) cnf_add_lit(cnf, vars[i]);
	cnf_add_lit(cnf, 0);
	cnf_end_clause(cnf);
	// at most one
	for (int i = 0; i < 9; i++) for (int j = i+1; j < 9; j++) {
		cnf_start_clause(cnf);
		cnf_add_lit(cnf, -vars[i]);
		cnf_add_lit(cnf, -vars[j]);
		cnf_add_lit(cnf, 0);
		cnf_end_clause(cnf);
	}
}

void windoku_add_diagonals(CNF *cnf) {
	for (int d = 1; d <= 9; d++) {
		int diag1[9], diag2[9];
		for (int i = 1; i <= 9; i++) diag1[i-1] = v(i,i,d);
		for (int i = 1; i <= 9; i++) diag2[i-1] = v(i,10-i,d);
		pairwise_all_diff(cnf, diag1);
		pairwise_all_diff(cnf, diag2);
	}
}

void windoku_add_windows(CNF *cnf) {
	// Windows: top-left anchored at (2,2), (2,6), (6,2), (6,6)
	int anchors[4][2] = { {2,2}, {2,6}, {6,2}, {6,6} };
	for (int w = 0; w < 4; w++) {
		int ar = anchors[w][0];
		int ac = anchors[w][1];
		for (int d = 1; d <= 9; d++) {
			int vars[9];
			int idx = 0;
			for (int dr = 0; dr < 3; dr++) for (int dc = 0; dc < 3; dc++) {
				vars[idx++] = v(ar+dr, ac+dc, d);
			}
			pairwise_all_diff(cnf, vars);
		}
	}
}

