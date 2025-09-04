#ifndef HEURISTICS_H
#define HEURISTICS_H

#include "cnf.h"

// Select next decision variable index in [1..num_vars], or 0 if none.
int heuristic_select_first(const CNF *cnf, const int *assignment);
int heuristic_select_max_occurrence(const CNF *cnf, const int *assignment);
int heuristic_select_jeroslow_wang(const CNF *cnf, const int *assignment);
int heuristic_select_random(const CNF *cnf, const int *assignment);

#endif // HEURISTICS_H
