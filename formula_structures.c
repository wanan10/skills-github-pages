#include "sat_solver_c.h"

// ===== 文字操作函数 =====
Literal createLiteral(int var, bool sign) {
    Literal lit;
    lit.var = var;
    lit.sign = sign;
    return lit;
}

Literal negateLiteral(Literal lit) {
    Literal negated;
    negated.var = lit.var;
    negated.sign = !lit.sign;
    return negated;
}

bool literalEquals(Literal a, Literal b) {
    return a.var == b.var && a.sign == b.sign;
}

// ===== 子句操作函数 =====
Clause createClause() {
    Clause clause;
    clause.literals.data = (Literal*)malloc(INITIAL_CAPACITY * sizeof(Literal));
    clause.literals.size = 0;
    clause.literals.capacity = INITIAL_CAPACITY;
    clause.satisfied = false;
    return clause;
}

void destroyClause(Clause* clause) {
    if (clause && clause->literals.data) {
        free(clause->literals.data);
        clause->literals.data = NULL;
    }
}

void clauseAddLiteral(Clause* clause, Literal lit) {
    literalArrayPush(&clause->literals, lit);
}

int clauseSize(const Clause* clause) {
    return clause->literals.size;
}

bool clauseIsEmpty(const Clause* clause) {
    return clause->literals.size == 0;
}

void clausePrint(const Clause* clause) {
    for (int i = 0; i < clause->literals.size; i++) {
        if (!clause->literals.data[i].sign) {
            printf("-");
        }
        printf("%d ", clause->literals.data[i].var);
    }
    printf("0\n");
}

// ===== CNF公式操作函数 =====
CNFFormula createCNFFormula() {
    CNFFormula formula;
    formula.clauses.data = (Clause*)malloc(INITIAL_CAPACITY * sizeof(Clause));
    formula.clauses.size = 0;
    formula.clauses.capacity = INITIAL_CAPACITY;
    formula.numVars = 0;
    formula.numClauses = 0;
    return formula;
}

void destroyCNFFormula(CNFFormula* formula) {
    if (formula && formula->clauses.data) {
        // 清理每个子句
        for (int i = 0; i < formula->clauses.size; i++) {
            destroyClause(&formula->clauses.data[i]);
        }
        free(formula->clauses.data);
        formula->clauses.data = NULL;
    }
}

void formulaAddClause(CNFFormula* formula, Clause clause) {
    clauseArrayPush(&formula->clauses, clause);
}

void formulaPrint(const CNFFormula* formula) {
    printf("p cnf %d %d\n", formula->numVars, formula->numClauses);
    for (int i = 0; i < formula->clauses.size; i++) {
        clausePrint(&formula->clauses.data[i]);
    }
}