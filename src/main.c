#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cnf.h"
#include "dpll.h"
#include "utils.h"

static void print_usage(const char *prog) {
	fprintf(stderr, "Usage: %s -i <input.cnf> [-o <out.txt>] [-H first|max|jw|rand] [--no-pure]\n", prog);
	fprintf(stderr, "       %s --validate -i <input.cnf>\n", prog);
}

static HeuristicType parse_heuristic(const char *s) {
	if (!s) return HEURISTIC_JEROSLOW_WANG;
	if (strcmp(s, "first") == 0) return HEURISTIC_FIRST;
	if (strcmp(s, "max") == 0) return HEURISTIC_MAX_OCCURRENCE;
	if (strcmp(s, "jw") == 0 || strcmp(s, "jeroslow-wang") == 0) return HEURISTIC_JEROSLOW_WANG;
	if (strcmp(s, "rand") == 0) return HEURISTIC_RANDOM;
	return HEURISTIC_JEROSLOW_WANG;
}

int main(int argc, char **argv) {
	const char *input = NULL;
	const char *output = NULL;
	int validate_only = 0;
	int use_pure = 1;
	HeuristicType heuristic = HEURISTIC_JEROSLOW_WANG;

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) { input = argv[++i]; }
		else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) { output = argv[++i]; }
		else if (strcmp(argv[i], "-H") == 0 && i + 1 < argc) { heuristic = parse_heuristic(argv[++i]); }
		else if (strcmp(argv[i], "--no-pure") == 0) { use_pure = 0; }
		else if (strcmp(argv[i], "--validate") == 0) { validate_only = 1; }
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) { print_usage(argv[0]); return 0; }
		else { /* ignore unknown for simplicity */ }
	}

	if (!input) { print_usage(argv[0]); return 1; }
	CNF *cnf = cnf_parse_dimacs(input);
	if (!cnf) { fprintf(stderr, "Failed to parse %s\n", input); return 2; }

	if (validate_only) {
		cnf_dump(cnf);
		cnf_free(cnf);
		return 0;
	}

	long long start = time_now_ms();
	DPLLResult res;
	memset(&res, 0, sizeof(res));
	if (dpll_solve(cnf, heuristic, use_pure, &res) != 0) {
		fprintf(stderr, "Solver error\n");
		cnf_free(cnf);
		return 3;
	}
	long long elapsed = time_elapsed_ms(start);
	res.time_ms = elapsed;

	FILE *out = output ? fopen(output, "w") : stdout;
	if (!out) { perror("fopen output"); out = stdout; }
	fprintf(out, "Result: %s\n", res.is_sat ? "SAT" : "UNSAT");
	fprintf(out, "Vars: %d, Clauses: %d\n", cnf->num_vars, cnf->num_clauses);
	fprintf(out, "Heuristic: %d, PureLiteral: %s\n", (int)heuristic, use_pure ? "on" : "off");
	fprintf(out, "Decisions: %d, Propagations: %lld\n", res.decisions, res.propagations);
	fprintf(out, "Time(ms): %lld\n", res.time_ms);
	if (res.is_sat && res.assignment) {
		fprintf(out, "Assignment:\n");
		for (int v = 1; v <= cnf->num_vars; ++v) {
			int val = res.assignment[v];
			if (val == -1) val = 1; // default
			fprintf(out, "%d ", val ? v : -v);
		}
		fprintf(out, "0\n");
	}
	if (out != stdout) fclose(out);

	free(res.assignment);
	cnf_free(cnf);
	return 0;
}

