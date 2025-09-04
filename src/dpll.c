#include "dpll.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

typedef struct {
	const CNF *cnf;
	int *assign; // -1,0,1 per var index 1..n
} Solver;

static int lit_value(const int *assign, int lit) {
	int var = lit > 0 ? lit : -lit;
	int val = assign[var];
	if (val == 0) return 0;
	if (lit > 0) return val;
	return -val;
}

static int unit_propagate(Solver *s) {
	int changed = 1;
	while (changed) {
		changed = 0;
		for (size_t i = 0; i < s->cnf->clauses.size; i++) {
			int *c = s->cnf->clauses.data[i];
			int any_true = 0;
			int num_unassigned = 0;
			int last_unassigned_lit = 0;
			for (int *p = c; *p != 0; ++p) {
				int v = lit_value(s->assign, *p);
				if (v > 0) { any_true = 1; break; }
				if (v == 0) { num_unassigned++; last_unassigned_lit = *p; }
			}
			if (any_true) continue;
			if (num_unassigned == 0) {
				return -1; // conflict
			}
			if (num_unassigned == 1) {
				int var = last_unassigned_lit > 0 ? last_unassigned_lit : -last_unassigned_lit;
				int val = last_unassigned_lit > 0 ? 1 : -1;
				if (s->assign[var] == 0) { s->assign[var] = val; changed = 1; }
			}
		}
	}
	return 0;
}

static int choose_literal(const Solver *s) {
	// simple heuristic: first unassigned variable as positive
	for (int v = 1; v <= s->cnf->num_vars; v++) if (s->assign[v] == 0) return v;
	return 0;
}

static int dpll_rec(Solver *s, DPLLResult *res) {
	int up = unit_propagate(s);
	if (up < 0) { res->conflicts++; return 0; }
	int lit = choose_literal(s);
	if (lit == 0) return 1; // all assigned -> SAT
	res->decisions++;
	// try true
	s->assign[lit] = 1;
	if (dpll_rec(s, res)) return 1;
	// backtrack and try false
	s->assign[lit] = -1;
	if (dpll_rec(s, res)) return 1;
	// backtrack
	s->assign[lit] = 0;
	return 0;
}

int dpll_solve(const CNF *cnf, DPLLResult *res, double *elapsed_ms) {
	struct timespec t0, t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	memset(res, 0, sizeof(*res));
	res->num_vars = cnf->num_vars;
	res->assignment = (int*)calloc((size_t)cnf->num_vars + 1, sizeof(int));
	if (!res->assignment) return -1;
	Solver s = { .cnf = cnf, .assign = res->assignment };
	int sat = dpll_rec(&s, res);
	clock_gettime(CLOCK_MONOTONIC, &t1);
	if (elapsed_ms) {
		double ms = (t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1e6;
		*elapsed_ms = ms;
	}
	return sat ? 1 : 0;
}

void dpll_result_free(DPLLResult *res) {
	free(res->assignment);
	res->assignment = NULL;
	res->num_vars = 0;
}

