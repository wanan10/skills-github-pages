#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

// 常量定义
#define MAX_LINE_LENGTH 1024
#define INITIAL_CAPACITY 8
#define MAX_VARIABLES 100000
#define MAX_CLAUSES 500000

// 赋值状态
typedef enum {
    UNASSIGNED = 0,
    SAT_TRUE = 1,
    SAT_FALSE = -1
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
    int id;  // 子句ID
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
    char* filename;
} CNFFormula;

// 求解统计信息
typedef struct {
    clock_t startTime;
    clock_t endTime;
    long long decisions;
    long long conflicts;
    long long propagations;
    long long backtracks;
} SolverStats;

// SAT求解器结构
typedef struct {
    CNFFormula* formula;
    int* assignment;                // 变量赋值数组
    IntArray* decisionStack;        // 决策栈
    IntArray* propagationQueue;     // 单元传播队列
    SolverStats stats;              // 统计信息
    bool useHeuristics;             // 是否使用启发式
} SATSolver;

// 数独网格结构
typedef struct {
    int grid[9][9];
    int originalGrid[9][9];  // 保存原始题目
    bool isXSudoku;          // 是否为X数独（对角线约束）
} SudokuGrid;

// 数独求解器结构
typedef struct {
    SudokuGrid* sudoku;
    SATSolver* satSolver;
    int difficulty;  // 难度级别
} SudokuSolver;

// 通用工具函数声明
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

// 文字和子句操作
Literal createLiteral(int var, bool sign);
Literal negateLiteral(Literal lit);
bool literalEquals(Literal a, Literal b);

Clause createClause(int id);
void destroyClause(Clause* clause);
void clauseAddLiteral(Clause* clause, Literal lit);
int clauseSize(const Clause* clause);
bool clauseIsEmpty(const Clause* clause);

// CNF公式操作
CNFFormula* cnfFormulaCreate();
void cnfFormulaDestroy(CNFFormula* formula);
void cnfFormulaAddClause(CNFFormula* formula, Clause clause);

// 统计信息操作
void statsInit(SolverStats* stats);
double statsGetSolvingTime(const SolverStats* stats);

#endif // COMMON_H