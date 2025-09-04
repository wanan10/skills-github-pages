#include "cnf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

void cnf_init(CNF *cnf) {
	intptrvec_init(&cnf->clauses);
	intvec_init(&cnf->storage);
	cnf->num_vars = 0;
}

void cnf_free(CNF *cnf) {
	intptrvec_free(&cnf->clauses);
	intvec_free(&cnf->storage);
	cnf->num_vars = 0;
}

void cnf_start_clause(CNF *cnf) {
	(void)cnf;
}

void cnf_add_lit(CNF *cnf, int lit) {
	intvec_push(&cnf->storage, lit);
}

void cnf_end_clause(CNF *cnf) {
	intvec_push(&cnf->storage, 0);
	// clause pointer is the start of this clause in storage
	// find last 0 and get pointer to its start
	// We store pointer to the clause's first literal
	// Compute start as address of last clause start by scanning back until 0 or beginning
	int *base = cnf->storage.data;
	size_t n = cnf->storage.size;
	if (n == 0) return;
	// Find start of this clause: walk back to previous 0 (exclusive)
	size_t i = n - 1; // points to 0
	while (i > 0 && base[i-1] != 0) i--;
	intptrvec_push(&cnf->clauses, &base[i]);
}

static int parse_int(const char **s, int *out) {
	while (isspace((unsigned char)**s)) (*s)++;
	int sign = 1;
	if (**s == '-') { sign = -1; (*s)++; }
	if (!isdigit((unsigned char)**s)) return 0;
	int v = 0;
	while (isdigit((unsigned char)**s)) { v = v*10 + (**s - '0'); (*s)++; }
	*out = sign * v;
	return 1;
}

int cnf_read_dimacs(CNF *cnf, const char *path) {
	size_t len = 0;
	char *txt = read_file_all(path, &len);
	if (!txt) {
		fprintf(stderr, "Failed to read %s\n", path);
		return -1;
	}
	const char *s = txt;
	int vars_declared = 0, clauses_declared = 0;
	while (*s) {
		while (isspace((unsigned char)*s)) s++;
		if (*s == 'c') { // comment line
			while (*s && *s != '\n') s++;
			continue;
		}
		if (*s == 'p') {
			// expect: p cnf <vars> <clauses>
			s += 1;
			while (isspace((unsigned char)*s)) s++;
			if (strncmp(s, "cnf", 3) != 0) { free(txt); return -1; }
			s += 3;
			int v=0,c=0;
			if (!parse_int(&s, &v)) { free(txt); return -1; }
			if (!parse_int(&s, &c)) { free(txt); return -1; }
			vars_declared = v;
			clauses_declared = c;
			continue;
		}
		// parse clause: sequence of ints ending with 0
		int lit=0;
		int saw_any = 0;
		cnf_start_clause(cnf);
		while (parse_int(&s, &lit)) {
			saw_any = 1;
			if (lit == 0) break;
			if (lit < 0) {
				int var = -lit;
				if (var > cnf->num_vars) cnf->num_vars = var;
			} else {
				if (lit > cnf->num_vars) cnf->num_vars = lit;
			}
			cnf_add_lit(cnf, lit);
		}
		if (saw_any) {
			cnf_end_clause(cnf);
		}
		// move to end of line
		while (*s && *s != '\n') s++;
	}
	if (vars_declared && cnf->num_vars < vars_declared) cnf->num_vars = vars_declared;
	(void)clauses_declared; // not strictly enforced
	free(txt);
	return 0;
}

int cnf_write_dimacs(const CNF *cnf, const char *path) {
	FILE *f = fopen(path, "wb");
	if (!f) return -1;
	fprintf(f, "p cnf %d %zu\n", cnf->num_vars, cnf->clauses.size);
	for (size_t i = 0; i < cnf->clauses.size; i++) {
		int *lit = cnf->clauses.data[i];
		for (; *lit != 0; ++lit) fprintf(f, "%d ", *lit);
		fprintf(f, "0\n");
	}
	fclose(f);
	return 0;
}

