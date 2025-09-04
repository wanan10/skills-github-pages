#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dpll.h"
#include "heuristics.h"
#include "utils.h"

typedef struct {
	const CNF *cnf;
	int *assignment; // size num_vars+1, -1 unassigned, 0 false, 1 true
	int *trail;      // decision/propagation trail (stack of literals assigned)
	int trail_sz;
	int trail_cap;
	int decisions;
	long long propagations;
} Solver;

static void trail_push(Solver *s, int lit) {
	if (s->trail_sz == s->trail_cap) {
		s->trail_cap = s->trail_cap ? s->trail_cap * 2 : 64;
		s->trail = (int *)realloc(s->trail, s->trail_cap * sizeof(int));
		if (!s->trail) { fprintf(stderr, "OOM trail\n"); exit(1); }
	}
	s->trail[s->trail_sz++] = lit;
}

static void backtrack(Solver *s, int mark_sz) {
	while (s->trail_sz > mark_sz) {
		int lit = s->trail[--s->trail_sz];
		int v = lit > 0 ? lit : -lit;
		s->assignment[v] = -1;
	}
}

static int clause_satisfied(const CNF *cnf, const int *assign, int ci) {
	int s = cnf->clause_offsets[ci];
	int e = cnf->clause_offsets[ci + 1];
	for (int j = s; j < e; ++j) {
		int lit = cnf->literals[j];
		int v = lit > 0 ? lit : -lit;
		int val = assign[v];
		if (val == -1) continue;
		if ((lit > 0 && val == 1) || (lit < 0 && val == 0)) return 1;
	}
	return 0;
}

static int clause_conflict_empty(const CNF *cnf, const int *assign, int ci) {
	int all_false = 1;
	int s = cnf->clause_offsets[ci];
	int e = cnf->clause_offsets[ci + 1];
	for (int j = s; j < e; ++j) {
		int lit = cnf->literals[j];
		int v = lit > 0 ? lit : -lit;
		int val = assign[v];
		if (val == -1) return 0; // unresolved
		if ((lit > 0 && val == 1) || (lit < 0 && val == 0)) return 0; // satisfied
	}
	return all_false;
}

static int unit_propagate(Solver *s) {
	int progress = 1;
	while (progress) {
		progress = 0;
		for (int i = 0; i < s->cnf->num_clauses; ++i) {
			if (clause_satisfied(s->cnf, s->assignment, i)) continue;
			int s_off = s->cnf->clause_offsets[i];
			int e_off = s->cnf->clause_offsets[i + 1];
			int unassigned_lit = 0;
			int num_unassigned = 0;
			int any_true = 0;
			for (int j = s_off; j < e_off; ++j) {
				int lit = s->cnf->literals[j];
				int v = lit > 0 ? lit : -lit;
				int val = s->assignment[v];
				if (val == -1) { num_unassigned++; unassigned_lit = lit; }
				else if ((lit > 0 && val == 1) || (lit < 0 && val == 0)) { any_true = 1; break; }
			}
			if (any_true) continue;
			if (num_unassigned == 0) return 0; // conflict
			if (num_unassigned == 1) {
				int v = unassigned_lit > 0 ? unassigned_lit : -unassigned_lit;
				int val = (unassigned_lit > 0) ? 1 : 0;
				if (s->assignment[v] == -1) {
					s->assignment[v] = val;
					trail_push(s, val ? v : -v);
					s->propagations++;
					progress = 1;
				} else {
					// check consistency
					if (s->assignment[v] != val) return 0; // conflict
				}
			}
		}
	}
	return 1; // no conflict
}

static void pure_literal_elimination(Solver *s) {
	int n = s->cnf->num_vars;
	char *seen_pos = (char *)calloc(n + 1, 1);
	char *seen_neg = (char *)calloc(n + 1, 1);
	for (int i = 0; i < s->cnf->num_clauses; ++i) {
		if (clause_satisfied(s->cnf, s->assignment, i)) continue;
		int s_off = s->cnf->clause_offsets[i];
		int e_off = s->cnf->clause_offsets[i + 1];
		for (int j = s_off; j < e_off; ++j) {
			int lit = s->cnf->literals[j];
			int v = lit > 0 ? lit : -lit;
			if (s->assignment[v] != -1) continue;
			if (lit > 0) seen_pos[v] = 1; else seen_neg[v] = 1;
		}
	}
	for (int v = 1; v <= n; ++v) {
		if (s->assignment[v] != -1) continue;
		if (seen_pos[v] && !seen_neg[v]) { s->assignment[v] = 1; trail_push(s, v); }
		else if (seen_neg[v] && !seen_pos[v]) { s->assignment[v] = 0; trail_push(s, -v); }
	}
	free(seen_pos); free(seen_neg);
}

static int all_assigned(const int *a, int n) {
	for (int v = 1; v <= n; ++v) {
		if (a[v] == -1) return 0;
	}
	return 1;
}

static int select_variable(const CNF *cnf, const int *assignment, HeuristicType h) {
	switch (h) {
		case HEURISTIC_MAX_OCCURRENCE: return heuristic_select_max_occurrence(cnf, assignment);
		case HEURISTIC_JEROSLOW_WANG: return heuristic_select_jeroslow_wang(cnf, assignment);
		case HEURISTIC_RANDOM: return heuristic_select_random(cnf, assignment);
		case HEURISTIC_FIRST:
		default: return heuristic_select_first(cnf, assignment);
	}
}

static int dpll_recursive(Solver *s, HeuristicType h, int use_pure) {
	if (!unit_propagate(s)) return 0; // conflict
	if (use_pure) pure_literal_elimination(s);
	if (all_assigned(s->assignment, s->cnf->num_vars)) return 1;
	int v = select_variable(s->cnf, s->assignment, h);
	if (v == 0) return 1;
	int mark = s->trail_sz;
	// try true first
	s->decisions++;
	s->assignment[v] = 1; trail_push(s, v);
	if (dpll_recursive(s, h, use_pure)) return 1;
	backtrack(s, mark);
	// then false
	s->assignment[v] = 0; trail_push(s, -v);
	if (dpll_recursive(s, h, use_pure)) return 1;
	backtrack(s, mark);
	return 0;
}

int dpll_solve(const CNF *cnf, HeuristicType heuristic, int use_pure, DPLLResult *out) {
	if (!cnf || !out) return -1;
	Solver s;
	memset(&s, 0, sizeof(s));
	s.cnf = cnf;
	s.assignment = (int *)malloc((cnf->num_vars + 1) * sizeof(int));
	if (!s.assignment) { fprintf(stderr, "OOM assignment\n"); return -1; }
	for (int i = 0; i <= cnf->num_vars; ++i) s.assignment[i] = -1;
	s.trail = NULL; s.trail_sz = 0; s.trail_cap = 0; s.decisions = 0; s.propagations = 0;

	int sat = dpll_recursive(&s, heuristic, use_pure);
	out->is_sat = sat;
	out->decisions = s.decisions;
	out->propagations = s.propagations;
	if (sat) {
		out->assignment = (int *)malloc((cnf->num_vars + 1) * sizeof(int));
		if (!out->assignment) { fprintf(stderr, "OOM result assignment\n"); free(s.assignment); free(s.trail); return -1; }
		memcpy(out->assignment, s.assignment, (cnf->num_vars + 1) * sizeof(int));
	} else {
		out->assignment = NULL;
	}
	free(s.assignment);
	free(s.trail);
	return 0;
}

