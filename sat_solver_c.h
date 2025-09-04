#ifndef SAT_SOLVER_C_H
#define SAT_SOLVER_C_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024
#define INITIAL_CAPACITY 8

// 赋值状态
typedef enum {
    UNASSIGNED = 0,
    TRUE_VAL = 1,
    FALSE_VAL = -1
} AssignmentState;

// 动态整数数组结构
typedef struct {
    int* data;
    int size;
    int capacity;
} IntArray;

// 文字结构
typedef struct {
    int var;      // 变量编号 (1, 2, 3, ...)
    bool sign;    // true表示正文字，false表示负文字
} Literal;

// 动态文字数组结构
typedef struct {
    Literal* data;
    int size;
    int capacity;
} LiteralArray;

// 子句结构
typedef struct {
    LiteralArray literals;
    bool satisfied;
} Clause;

// 动态子句数组结构
typedef struct {
    Clause* data;
    int size;
    int capacity;
} ClauseArray;

// CNF公式结构
typedef struct {
    ClauseArray clauses;
    int numVars;
    int numClauses;
} CNFFormula;

// SAT求解器结构
typedef struct {
    CNFFormula formula;
    int* assignment;                // 变量赋值数组
    IntArray decisionStack;         // 决策栈
    IntArray propagationQueue;      // 单元传播队列
    
    // 性能统计
    clock_t startTime;
    clock_t endTime;
    long long decisions;
    long long conflicts;
    long long propagations;
    
    // 优化选项
    bool useHeuristics;
} SATSolver;

// 数独求解器结构
typedef struct {
    int grid[9][9];
    SATSolver* satSolver;
} SudokuSolver;

// 动态数组函数声明
IntArray* intArrayCreate();
void intArrayDestroy(IntArray* arr);
void intArrayPush(IntArray* arr, int value);
int intArrayGet(IntArray* arr, int index);
void intArrayPop(IntArray* arr);
void intArrayClear(IntArray* arr);
bool intArrayEmpty(IntArray* arr);

LiteralArray* literalArrayCreate();
void literalArrayDestroy(LiteralArray* arr);
void literalArrayPush(LiteralArray* arr, Literal lit);
Literal literalArrayGet(LiteralArray* arr, int index);

ClauseArray* clauseArrayCreate();
void clauseArrayDestroy(ClauseArray* arr);
void clauseArrayPush(ClauseArray* arr, Clause clause);
Clause* clauseArrayGetPtr(ClauseArray* arr, int index);

// 文字和子句函数
Literal createLiteral(int var, bool sign);
Literal negateLiteral(Literal lit);
bool literalEquals(Literal a, Literal b);

Clause createClause();
void destroyClause(Clause* clause);
void clauseAddLiteral(Clause* clause, Literal lit);
int clauseSize(const Clause* clause);
bool clauseIsEmpty(const Clause* clause);
void clausePrint(const Clause* clause);

// CNF公式函数
CNFFormula createCNFFormula();
void destroyCNFFormula(CNFFormula* formula);
void formulaAddClause(CNFFormula* formula, Clause clause);
void formulaPrint(const CNFFormula* formula);

// SAT求解器函数
SATSolver* satSolverCreate();
void satSolverDestroy(SATSolver* solver);
bool satSolverLoadCNF(SATSolver* solver, const char* filename);
void satSolverPrintFormula(const SATSolver* solver);
bool satSolverSolve(SATSolver* solver, bool useOptimization);
void satSolverPrintSolution(const SATSolver* solver);
void satSolverPrintStatistics(const SATSolver* solver);
double satSolverGetSolvingTime(const SATSolver* solver);
void satSolverSaveSolution(const SATSolver* solver, const char* filename);
bool satSolverVerifySolution(const SATSolver* solver);
int satSolverGetNumVars(const SATSolver* solver);
int satSolverGetNumClauses(const SATSolver* solver);

// DPLL算法内部函数
bool satSolverUnitPropagate(SATSolver* solver);
int satSolverSelectBranchingVariable(SATSolver* solver);
int satSolverSelectBranchingVariableWithHeuristic(SATSolver* solver);
bool satSolverIsFormulaUnsatisfied(const SATSolver* solver);
bool satSolverIsFormulaSatisfied(const SATSolver* solver);
void satSolverAssignLiteral(SATSolver* solver, int var, bool value);
void satSolverUnassignVariable(SATSolver* solver, int var);
bool satSolverDPLL(SATSolver* solver);

// 数独求解器函数
SudokuSolver* sudokuSolverCreate();
void sudokuSolverDestroy(SudokuSolver* solver);
void sudokuSolverLoadFromFile(SudokuSolver* solver, const char* filename);
void sudokuSolverLoadFromInput(SudokuSolver* solver);
void sudokuSolverPrintGrid(const SudokuSolver* solver);
bool sudokuSolverSolve(SudokuSolver* solver);
void sudokuSolverPlayInteractiveGame(SudokuSolver* solver);
void sudokuSolverGenerateSATClauses(SudokuSolver* solver);
int sudokuSolverGetVarIndex(int row, int col, int num);
void sudokuSolverExtractSolution(SudokuSolver* solver);

// 工具函数
void printUsage();
void runPerformanceComparison(const char* filename);

#endif // SAT_SOLVER_C_H