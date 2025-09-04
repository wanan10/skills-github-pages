#ifndef DPLL_H
#define DPLL_H

#include "cnf.h"

typedef enum {
	HEURISTIC_FIRST = 0,
	HEURISTIC_MAX_OCCURRENCE = 1,
	HEURISTIC_JEROSLOW_WANG = 2,
	HEURISTIC_RANDOM = 3
} HeuristicType;

typedef struct {
	int is_sat;          // 1 SAT, 0 UNSAT
	int *assignment;     // size num_vars+1, values: -1 unassigned, 0 false, 1 true
	long long time_ms;   // execution time in milliseconds (filled by caller optionally)
	int decisions;
	long long propagations;
} DPLLResult;

// Solve CNF using DPLL. heuristic selects variable selection strategy.
// use_pure_literal controls whether to perform pure literal elimination.
// Returns 0 on success, non-zero on error. DPLLResult.assignment is allocated on success for SAT.
int dpll_solve(const CNF *cnf, HeuristicType heuristic, int use_pure_literal, DPLLResult *out_result);

#endif // DPLL_H
