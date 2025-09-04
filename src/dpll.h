#ifndef DPLL_H
#define DPLL_H

#include "cnf.h"
#include <time.h>

typedef struct {
	int *assignment; // 1..num_vars, values: -1=false, 1=true, 0=unassigned
	int num_vars;
	// stats
	unsigned long decisions;
	unsigned long propagations;
	unsigned long conflicts;
} DPLLResult;

int dpll_solve(const CNF *cnf, DPLLResult *res, double *elapsed_ms);
void dpll_result_free(DPLLResult *res);

#endif

