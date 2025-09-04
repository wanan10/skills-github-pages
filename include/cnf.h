#ifndef CNF_H
#define CNF_H

#include <stddef.h>

typedef struct {
	int num_vars;
	int num_clauses;
	int *clause_offsets; // length = num_clauses + 1
	int *literals;       // flat literals array, clause i spans [offsets[i], offsets[i+1])
} CNF;

// Parse a DIMACS CNF file from path. Returns NULL on failure.
CNF *cnf_parse_dimacs(const char *path);

// Dump CNF clauses to stdout for manual validation.
void cnf_dump(const CNF *cnf);

// Free CNF resources.
void cnf_free(CNF *cnf);

// Builder API for programmatic CNF creation (e.g., Sudoku encoding)
typedef struct {
	int num_vars;
	int num_clauses;
	int capacity_clauses;
	int capacity_literals;
	int *clause_offsets; // size capacity_clauses + 1
	int *literals;       // size capacity_literals
	int literals_used;
} CNFBuilder;

void cnf_builder_init(CNFBuilder *builder, int num_vars, int clause_cap, int lit_cap);
void cnf_builder_add_clause(CNFBuilder *builder, const int *lits, int len);
CNF *cnf_builder_finalize(CNFBuilder *builder);
void cnf_builder_free(CNFBuilder *builder);

#endif // CNF_H
