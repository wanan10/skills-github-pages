#ifndef CNF_H
#define CNF_H

#include <stddef.h>
#include "util.h"

typedef struct {
	// Clauses are sequences of ints ending with 0
	IntPtrVec clauses;
	IntVec storage; // flat storage for all literals with 0 separators
	int num_vars;
} CNF;

void cnf_init(CNF *cnf);
void cnf_free(CNF *cnf);

// DIMACS parser: returns 0 on success
int cnf_read_dimacs(CNF *cnf, const char *path);
int cnf_write_dimacs(const CNF *cnf, const char *path);

// Builder helpers
void cnf_start_clause(CNF *cnf);
void cnf_add_lit(CNF *cnf, int lit);
void cnf_end_clause(CNF *cnf);

#endif

