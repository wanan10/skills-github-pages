#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cnf.h"

static void *xmalloc(size_t n) {
	void *p = malloc(n);
	if (!p) {
		fprintf(stderr, "Out of memory allocating %zu bytes\n", n);
		exit(1);
	}
	return p;
}

static void *xrealloc(void *ptr, size_t n) {
	void *p = realloc(ptr, n);
	if (!p) {
		fprintf(stderr, "Out of memory reallocating %zu bytes\n", n);
		exit(1);
	}
	return p;
}

CNF *cnf_parse_dimacs(const char *path) {
	FILE *f = fopen(path, "r");
	if (!f) {
		perror("fopen");
		return NULL;
	}

	int capacity_clauses = 1024;
	int capacity_literals = 4096;
	int *clause_offsets = (int *)xmalloc((capacity_clauses + 1) * sizeof(int));
	int *literals = (int *)xmalloc(capacity_literals * sizeof(int));
	int declared_num_vars = 0, declared_num_clauses = 0;
	int clause_count = 0, lits_used = 0;

	char line[1 << 15];
	int header_seen = 0;
	while (fgets(line, sizeof(line), f)) {
		char *p = line;
		while (isspace((unsigned char)*p)) p++;
		if (*p == '\0' || *p == 'c' || *p == 'C' || *p == '%') continue;
		if (*p == 'p') {
			// Problem line
			char fmt[16];
			if (sscanf(p, "p %15s %d %d", fmt, &declared_num_vars, &declared_num_clauses) != 3) {
				fprintf(stderr, "Invalid DIMACS header: %s", line);
				fclose(f);
				free(clause_offsets);
				free(literals);
				return NULL;
			}
			header_seen = 1;
			if (declared_num_clauses + 1 > capacity_clauses) {
				capacity_clauses = declared_num_clauses + 1;
				clause_offsets = (int *)xrealloc(clause_offsets, (capacity_clauses + 1) * sizeof(int));
			}
			clause_offsets[0] = 0;
			continue;
		}
		if (!header_seen) continue; // ignore leading comments before header

		// Parse clause line(s)
		int lit;
		char *q = p;
		while (1) {
			while (isspace((unsigned char)*q)) q++;
			if (*q == '\0' || *q == '\n') break;
			int consumed = 0;
			if (sscanf(q, "%d%n", &lit, &consumed) == 1) {
				q += consumed;
				if (lit == 0) {
					// end of clause
					if (clause_count + 2 > capacity_clauses) {
						capacity_clauses *= 2;
						clause_offsets = (int *)xrealloc(clause_offsets, (capacity_clauses + 1) * sizeof(int));
					}
					clause_offsets[clause_count + 1] = lits_used;
					clause_count++;
					continue;
				}
				if (lits_used + 1 > capacity_literals) {
					capacity_literals *= 2;
					literals = (int *)xrealloc(literals, capacity_literals * sizeof(int));
				}
				literals[lits_used++] = lit;
			} else {
				break;
			}
		}
	}

	fclose(f);

	CNF *cnf = (CNF *)xmalloc(sizeof(CNF));
	cnf->num_vars = declared_num_vars > 0 ? declared_num_vars : 0;
	cnf->num_clauses = clause_count;
	cnf->clause_offsets = clause_offsets;
	cnf->literals = literals;
	return cnf;
}

void cnf_dump(const CNF *cnf) {
	if (!cnf) return;
	printf("p cnf %d %d\n", cnf->num_vars, cnf->num_clauses);
	for (int i = 0; i < cnf->num_clauses; ++i) {
		int start = cnf->clause_offsets[i];
		int end = cnf->clause_offsets[i + 1];
		for (int j = start; j < end; ++j) {
			printf("%d ", cnf->literals[j]);
		}
		printf("0\n");
	}
}

void cnf_free(CNF *cnf) {
	if (!cnf) return;
	free(cnf->clause_offsets);
	free(cnf->literals);
	free(cnf);
}

void cnf_builder_init(CNFBuilder *b, int num_vars, int clause_cap, int lit_cap) {
	b->num_vars = num_vars;
	b->num_clauses = 0;
	b->capacity_clauses = clause_cap > 0 ? clause_cap : 16;
	b->capacity_literals = lit_cap > 0 ? lit_cap : 64;
	b->clause_offsets = (int *)xmalloc((b->capacity_clauses + 1) * sizeof(int));
	b->literals = (int *)xmalloc(b->capacity_literals * sizeof(int));
	b->literals_used = 0;
	b->clause_offsets[0] = 0;
}

void cnf_builder_add_clause(CNFBuilder *b, const int *lits, int len) {
	if (b->num_clauses + 2 > b->capacity_clauses) {
		b->capacity_clauses *= 2;
		b->clause_offsets = (int *)xrealloc(b->clause_offsets, (b->capacity_clauses + 1) * sizeof(int));
	}
	if (b->literals_used + len > b->capacity_literals) {
		while (b->literals_used + len > b->capacity_literals) b->capacity_literals *= 2;
		b->literals = (int *)xrealloc(b->literals, b->capacity_literals * sizeof(int));
	}
	for (int i = 0; i < len; ++i) b->literals[b->literals_used++] = lits[i];
	b->clause_offsets[b->num_clauses + 1] = b->literals_used;
	b->num_clauses++;
}

CNF *cnf_builder_finalize(CNFBuilder *b) {
	CNF *cnf = (CNF *)xmalloc(sizeof(CNF));
	cnf->num_vars = b->num_vars;
	cnf->num_clauses = b->num_clauses;
	cnf->clause_offsets = b->clause_offsets;
	cnf->literals = b->literals;
	// transfer ownership
	b->clause_offsets = NULL;
	b->literals = NULL;
	return cnf;
}

void cnf_builder_free(CNFBuilder *b) {
	if (!b) return;
	free(b->clause_offsets);
	free(b->literals);
}

