#include "common.h"

// ===== 整数动态数组实现 =====
IntArray* intArrayCreate() {
    IntArray* arr = (IntArray*)malloc(sizeof(IntArray));
    if (!arr) return NULL;
    
    arr->data = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    if (!arr->data) {
        free(arr);
        return NULL;
    }
    
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
    return arr;
}

void intArrayDestroy(IntArray* arr) {
    if (arr) {
        if (arr->data) {
            free(arr->data);
        }
        free(arr);
    }
}

static void intArrayResize(IntArray* arr) {
    int newCapacity = arr->capacity * 2;
    int* newData = (int*)realloc(arr->data, newCapacity * sizeof(int));
    if (newData) {
        arr->data = newData;
        arr->capacity = newCapacity;
    }
}

void intArrayPush(IntArray* arr, int value) {
    if (arr->size >= arr->capacity) {
        intArrayResize(arr);
    }
    arr->data[arr->size++] = value;
}

int intArrayGet(IntArray* arr, int index) {
    if (index >= 0 && index < arr->size) {
        return arr->data[index];
    }
    return 0;
}

void intArrayPop(IntArray* arr) {
    if (arr->size > 0) {
        arr->size--;
    }
}

void intArrayClear(IntArray* arr) {
    arr->size = 0;
}

bool intArrayEmpty(IntArray* arr) {
    return arr->size == 0;
}

// ===== 文字动态数组实现 =====
LiteralArray* literalArrayCreate() {
    LiteralArray* arr = (LiteralArray*)malloc(sizeof(LiteralArray));
    if (!arr) return NULL;
    
    arr->data = (Literal*)malloc(INITIAL_CAPACITY * sizeof(Literal));
    if (!arr->data) {
        free(arr);
        return NULL;
    }
    
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
    return arr;
}

void literalArrayDestroy(LiteralArray* arr) {
    if (arr) {
        if (arr->data) {
            free(arr->data);
        }
        free(arr);
    }
}

static void literalArrayResize(LiteralArray* arr) {
    int newCapacity = arr->capacity * 2;
    Literal* newData = (Literal*)realloc(arr->data, newCapacity * sizeof(Literal));
    if (newData) {
        arr->data = newData;
        arr->capacity = newCapacity;
    }
}

void literalArrayPush(LiteralArray* arr, Literal lit) {
    if (arr->size >= arr->capacity) {
        literalArrayResize(arr);
    }
    arr->data[arr->size++] = lit;
}

Literal literalArrayGet(LiteralArray* arr, int index) {
    if (index >= 0 && index < arr->size) {
        return arr->data[index];
    }
    Literal empty = {0, true};
    return empty;
}

// ===== 子句动态数组实现 =====
ClauseArray* clauseArrayCreate() {
    ClauseArray* arr = (ClauseArray*)malloc(sizeof(ClauseArray));
    if (!arr) return NULL;
    
    arr->data = (Clause*)malloc(INITIAL_CAPACITY * sizeof(Clause));
    if (!arr->data) {
        free(arr);
        return NULL;
    }
    
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
    return arr;
}

void clauseArrayDestroy(ClauseArray* arr) {
    if (arr) {
        if (arr->data) {
            for (int i = 0; i < arr->size; i++) {
                destroyClause(&arr->data[i]);
            }
            free(arr->data);
        }
        free(arr);
    }
}

static void clauseArrayResize(ClauseArray* arr) {
    int newCapacity = arr->capacity * 2;
    Clause* newData = (Clause*)realloc(arr->data, newCapacity * sizeof(Clause));
    if (newData) {
        arr->data = newData;
        arr->capacity = newCapacity;
    }
}

void clauseArrayPush(ClauseArray* arr, Clause clause) {
    if (arr->size >= arr->capacity) {
        clauseArrayResize(arr);
    }
    arr->data[arr->size++] = clause;
}

Clause* clauseArrayGetPtr(ClauseArray* arr, int index) {
    if (index >= 0 && index < arr->size) {
        return &arr->data[index];
    }
    return NULL;
}

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
Clause createClause(int id) {
    Clause clause;
    clause.literals.data = (Literal*)malloc(INITIAL_CAPACITY * sizeof(Literal));
    clause.literals.size = 0;
    clause.literals.capacity = INITIAL_CAPACITY;
    clause.satisfied = false;
    clause.id = id;
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

// ===== CNF公式操作函数 =====
CNFFormula* cnfFormulaCreate() {
    CNFFormula* formula = (CNFFormula*)malloc(sizeof(CNFFormula));
    if (!formula) return NULL;
    
    formula->clauses.data = (Clause*)malloc(INITIAL_CAPACITY * sizeof(Clause));
    formula->clauses.size = 0;
    formula->clauses.capacity = INITIAL_CAPACITY;
    formula->numVars = 0;
    formula->numClauses = 0;
    formula->filename = NULL;
    return formula;
}

void cnfFormulaDestroy(CNFFormula* formula) {
    if (formula) {
        if (formula->clauses.data) {
            for (int i = 0; i < formula->clauses.size; i++) {
                destroyClause(&formula->clauses.data[i]);
            }
            free(formula->clauses.data);
        }
        if (formula->filename) {
            free(formula->filename);
        }
        free(formula);
    }
}

void cnfFormulaAddClause(CNFFormula* formula, Clause clause) {
    clauseArrayPush(&formula->clauses, clause);
}

// ===== 统计信息操作 =====
void statsInit(SolverStats* stats) {
    stats->startTime = 0;
    stats->endTime = 0;
    stats->decisions = 0;
    stats->conflicts = 0;
    stats->propagations = 0;
    stats->backtracks = 0;
}

double statsGetSolvingTime(const SolverStats* stats) {
    return ((double)(stats->endTime - stats->startTime) / CLOCKS_PER_SEC) * 1000.0;
}