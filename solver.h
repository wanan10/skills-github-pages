#ifndef SOLVER_H
#define SOLVER_H

#include "common.h"

// 求解结果枚举
typedef enum {
    SOLVER_UNKNOWN = 0,
    SOLVER_SAT = 1,
    SOLVER_UNSAT = -1,
    SOLVER_TIMEOUT = 2,
    SOLVER_ERROR = -2
} SolverResult;

// 启发式策略枚举
typedef enum {
    HEURISTIC_NONE = 0,
    HEURISTIC_FIRST_UNASSIGNED = 1,
    HEURISTIC_VSIDS = 2,
    HEURISTIC_RANDOM = 3
} HeuristicType;

// DPLL求解器函数声明
SATSolver* solverCreate();
void solverDestroy(SATSolver* solver);

// 求解器初始化和配置
bool solverLoadFormula(SATSolver* solver, CNFFormula* formula);
void solverSetHeuristic(SATSolver* solver, HeuristicType heuristic);
void solverReset(SATSolver* solver);

// 核心求解函数
SolverResult solverSolve(SATSolver* solver);
bool solverDPLL(SATSolver* solver);

// DPLL算法组件
bool solverUnitPropagate(SATSolver* solver);
int solverSelectBranchingVariable(SATSolver* solver);
bool solverAssignVariable(SATSolver* solver, int var, bool value);
void solverUnassignVariable(SATSolver* solver, int var);
bool solverBacktrack(SATSolver* solver);

// 启发式函数
int solverHeuristicFirstUnassigned(SATSolver* solver);
int solverHeuristicVSIDS(SATSolver* solver);
int solverHeuristicRandom(SATSolver* solver);

// 公式状态检查
bool solverIsFormulaSatisfied(const SATSolver* solver);
bool solverIsFormulaUnsatisfied(const SATSolver* solver);
bool solverHasConflict(const SATSolver* solver);

// 解验证和输出
bool solverVerifySolution(const SATSolver* solver);
void solverPrintSolution(const SATSolver* solver);
void solverPrintStatistics(const SATSolver* solver);
void solverSaveSolution(const SATSolver* solver, const char* filename);

// 性能分析
double solverGetSolvingTime(const SATSolver* solver);
void solverStartTiming(SATSolver* solver);
void solverStopTiming(SATSolver* solver);

// 调试和诊断
void solverPrintCurrentState(const SATSolver* solver);
void solverPrintDecisionStack(const SATSolver* solver);
bool solverCheckConsistency(const SATSolver* solver);

#endif // SOLVER_H