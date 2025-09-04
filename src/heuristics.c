#include <stdlib.h>
#include <string.h>
#include "heuristics.h"

static int first_unassigned(const int *assignment, int n) {
	for (int v = 1; v <= n; ++v) if (assignment[v] == -1) return v;
	return 0;
}

int heuristic_select_first(const CNF *cnf, const int *assignment) {
	return first_unassigned(assignment, cnf->num_vars);
}

int heuristic_select_max_occurrence(const CNF *cnf, const int *assignment) {
	int n = cnf->num_vars;
	int *count = (int *)calloc((size_t)(n + 1), sizeof(int));
	if (!count) return 0;
	for (int i = 0; i < cnf->num_clauses; ++i) {
		int s = cnf->clause_offsets[i];
		int e = cnf->clause_offsets[i + 1];
		for (int j = s; j < e; ++j) {
			int v = cnf->literals[j];
			if (v < 0) v = -v;
			count[v]++;
		}
	}
	int best_v = 0, best_c = -1;
	for (int v = 1; v <= n; ++v) {
		if (assignment[v] == -1 && count[v] > best_c) { best_c = count[v]; best_v = v; }
	}
	free(count);
	return best_v;
}

int heuristic_select_jeroslow_wang(const CNF *cnf, const int *assignment) {
	int n = cnf->num_vars;
	double *score = (double *)malloc((size_t)(n + 1) * sizeof(double));
	if (!score) return 0;
	for (int i = 0; i <= n; ++i) score[i] = 0.0;
	for (int i = 0; i < cnf->num_clauses; ++i) {
		int s = cnf->clause_offsets[i];
		int e = cnf->clause_offsets[i + 1];
		int len = e - s;
		double w = 1.0;
		for (int k = 0; k < len; ++k) w *= 0.5; // 2^{-len}
		for (int j = s; j < e; ++j) {
			int v = cnf->literals[j];
			if (v < 0) v = -v;
			score[v] += w;
		}
	}
	int best_v = 0; double best_s = -1.0;
	for (int v = 1; v <= n; ++v) if (assignment[v] == -1 && score[v] > best_s) { best_s = score[v]; best_v = v; }
	free(score);
	return best_v;
}

int heuristic_select_random(const CNF *cnf, const int *assignment) {
	int n = cnf->num_vars;
	int remaining = 0;
	for (int v = 1; v <= n; ++v) if (assignment[v] == -1) remaining++;
	if (!remaining) return 0;
	int target = rand() % remaining;
	for (int v = 1; v <= n; ++v) if (assignment[v] == -1) { if (target-- == 0) return v; }
	return 0;
}

