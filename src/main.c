#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cnf.h"
#include "dpll.h"
#include "sudoku.h"
#include "windoku.h"

static void usage(void) {
	printf("Usage:\n");
	printf("  sat_sudoku --sat <cnf_file>\n");
	printf("  sat_sudoku --sudoku <grid81> [--windoku] [--diag] [--windows] [--dump <out.cnf>]\n");
	printf("  sat_sudoku --help\n");
}

static void print_sudoku_solution(const int *assign) {
	for (int r = 1; r <= 9; r++) {
		for (int c = 1; c <= 9; c++) {
			int digit = 0;
			for (int d = 1; d <= 9; d++) {
				if (assign[v(r,c,d)] > 0) { digit = d; break; }
			}
			printf("%d", digit);
		}
		printf("\n");
	}
}

int main(int argc, char **argv) {
	if (argc < 2) { usage(); return 1; }
	if (!strcmp(argv[1], "--help")) { usage(); return 0; }
	if (!strcmp(argv[1], "--sat")) {
		if (argc < 3) { usage(); return 1; }
		const char *cnf_path = argv[2];
		CNF cnf; cnf_init(&cnf);
		if (cnf_read_dimacs(&cnf, cnf_path) != 0) { fprintf(stderr, "Parse error\n"); return 1; }
		DPLLResult res; double ms=0.0;
		int sat = dpll_solve(&cnf, &res, &ms);
		printf("Result: %s\n", sat?"SAT":"UNSAT");
		printf("Time: %.3f ms\n", ms);
		if (sat) {
			// print model: first 20 vars
			int shown = 0;
			for (int i = 1; i <= cnf.num_vars && shown < 20; i++) {
				printf("x%d=%d ", i, res.assignment[i]);
				shown++;
			}
			printf("\n");
		}
		dpll_result_free(&res);
		cnf_free(&cnf);
		return 0;
	}
	if (!strcmp(argv[1], "--sudoku")) {
		if (argc < 3) { usage(); return 1; }
		const char *grid81 = argv[2];
		int add_windoku = 0, add_diag = 0, add_windows = 0;
		const char *dump = NULL;
		for (int i = 3; i < argc; i++) {
			if (!strcmp(argv[i], "--windoku")) add_windoku = 1, add_diag = 1, add_windows = 1;
			else if (!strcmp(argv[i], "--diag")) add_diag = 1;
			else if (!strcmp(argv[i], "--windows")) add_windows = 1;
			else if (!strcmp(argv[i], "--dump") && i+1 < argc) { dump = argv[++i]; }
		}
		CNF cnf; cnf_init(&cnf);
		sudoku_add_base_rules(&cnf);
		if (sudoku_add_clues(&cnf, grid81) != 0) { fprintf(stderr, "bad grid81\n"); return 1; }
		if (add_diag) windoku_add_diagonals(&cnf);
		if (add_windows) windoku_add_windows(&cnf);
		if (dump) cnf_write_dimacs(&cnf, dump);
		DPLLResult res; double ms=0.0;
		int sat = dpll_solve(&cnf, &res, &ms);
		printf("Result: %s\n", sat?"SAT":"UNSAT");
		printf("Time: %.3f ms\n", ms);
		if (sat) print_sudoku_solution(res.assignment);
		dpll_result_free(&res);
		cnf_free(&cnf);
		return 0;
	}
	usage();
	return 1;
}

