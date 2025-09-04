#include "solver.h"

// ===== 求解器创建和销毁 =====
SATSolver* solverCreate() {
    SATSolver* solver = (SATSolver*)malloc(sizeof(SATSolver));
    if (!solver) return NULL;
    
    solver->formula = NULL;
    solver->assignment = NULL;
    solver->decisionStack = intArrayCreate();
    solver->propagationQueue = intArrayCreate();
    
    if (!solver->decisionStack || !solver->propagationQueue) {
        solverDestroy(solver);
        return NULL;
    }
    
    statsInit(&solver->stats);
    solver->useHeuristics = false;
    
    return solver;
}

void solverDestroy(SATSolver* solver) {
    if (solver) {
        if (solver->assignment) {
            free(solver->assignment);
        }
        if (solver->decisionStack) {
            intArrayDestroy(solver->decisionStack);
        }
        if (solver->propagationQueue) {
            intArrayDestroy(solver->propagationQueue);
        }
        free(solver);
    }
}

// ===== 求解器初始化 =====
bool solverLoadFormula(SATSolver* solver, CNFFormula* formula) {
    if (!solver || !formula) {
        return false;
    }
    
    solver->formula = formula;
    
    // 初始化赋值数组
    if (solver->assignment) {
        free(solver->assignment);
    }
    
    solver->assignment = (int*)calloc(formula->numVars + 1, sizeof(int));
    if (!solver->assignment) {
        return false;
    }
    
    // 初始化为未赋值状态
    for (int i = 0; i <= formula->numVars; i++) {
        solver->assignment[i] = UNASSIGNED;
    }
    
    solverReset(solver);
    return true;
}

void solverSetHeuristic(SATSolver* solver, HeuristicType heuristic) {
    if (solver) {
        solver->useHeuristics = (heuristic != HEURISTIC_NONE);
    }
}

void solverReset(SATSolver* solver) {
    if (!solver) return;
    
    // 重置赋值
    if (solver->assignment && solver->formula) {
        for (int i = 1; i <= solver->formula->numVars; i++) {
            solver->assignment[i] = UNASSIGNED;
        }
    }
    
    // 重置子句状态
    if (solver->formula) {
        for (int i = 0; i < solver->formula->clauses.size; i++) {
            solver->formula->clauses.data[i].satisfied = false;
        }
    }
    
    // 清空栈和队列
    if (solver->decisionStack) {
        intArrayClear(solver->decisionStack);
    }
    if (solver->propagationQueue) {
        intArrayClear(solver->propagationQueue);
    }
    
    // 重置统计信息
    statsInit(&solver->stats);
}

// ===== 核心求解函数 =====
SolverResult solverSolve(SATSolver* solver) {
    if (!solver || !solver->formula) {
        return SOLVER_ERROR;
    }
    
    solverStartTiming(solver);
    bool result = solverDPLL(solver);
    solverStopTiming(solver);
    
    return result ? SOLVER_SAT : SOLVER_UNSAT;
}

bool solverDPLL(SATSolver* solver) {
    // 单元传播
    if (!solverUnitPropagate(solver)) {
        solver->stats.conflicts++;
        return false;
    }
    
    // 检查是否所有子句都满足
    if (solverIsFormulaSatisfied(solver)) {
        return true;
    }
    
    // 选择分支变量
    int var = solverSelectBranchingVariable(solver);
    if (var == 0) {
        return solverIsFormulaSatisfied(solver);
    }
    
    solver->stats.decisions++;
    
    // 尝试赋值为true
    intArrayPush(solver->decisionStack, var);
    if (solverAssignVariable(solver, var, true)) {
        if (solverDPLL(solver)) {
            return true;
        }
    }
    
    // 回溯并尝试赋值为false
    solverUnassignVariable(solver, var);
    if (solverAssignVariable(solver, var, false)) {
        if (solverDPLL(solver)) {
            return true;
        }
    }
    
    // 两种赋值都失败，回溯
    solverUnassignVariable(solver, var);
    intArrayPop(solver->decisionStack);
    solver->stats.backtracks++;
    
    // 重置子句满足状态
    for (int i = 0; i < solver->formula->clauses.size; i++) {
        solver->formula->clauses.data[i].satisfied = false;
    }
    
    return false;
}

// ===== 单元传播 =====
bool solverUnitPropagate(SATSolver* solver) {
    bool propagated = false;
    
    do {
        propagated = false;
        
        for (int i = 0; i < solver->formula->clauses.size; i++) {
            Clause* clause = &solver->formula->clauses.data[i];
            if (clause->satisfied) continue;
            
            int unassignedCount = 0;
            int unassignedVar = 0;
            bool unassignedSign = true;
            bool clauseSatisfied = false;
            
            // 检查子句状态
            for (int j = 0; j < clause->literals.size; j++) {
                const Literal* lit = &clause->literals.data[j];
                
                if (solver->assignment[lit->var] == UNASSIGNED) {
                    unassignedCount++;
                    unassignedVar = lit->var;
                    unassignedSign = lit->sign;
                } else if ((solver->assignment[lit->var] == SAT_TRUE && lit->sign) || 
                          (solver->assignment[lit->var] == SAT_FALSE && !lit->sign)) {
                    clauseSatisfied = true;
                    clause->satisfied = true;
                    break;
                }
            }
            
            if (clauseSatisfied) continue;
            
            if (unassignedCount == 0) {
                // 子句不满足，产生冲突
                return false;
            } else if (unassignedCount == 1) {
                // 单元子句，进行传播
                solver->assignment[unassignedVar] = unassignedSign ? SAT_TRUE : SAT_FALSE;
                intArrayPush(solver->propagationQueue, unassignedVar);
                propagated = true;
                solver->stats.propagations++;
            }
        }
    } while (propagated);
    
    return true;
}

// ===== 变量选择 =====
int solverSelectBranchingVariable(SATSolver* solver) {
    if (solver->useHeuristics) {
        return solverHeuristicVSIDS(solver);
    } else {
        return solverHeuristicFirstUnassigned(solver);
    }
}

int solverHeuristicFirstUnassigned(SATSolver* solver) {
    for (int i = 1; i <= solver->formula->numVars; i++) {
        if (solver->assignment[i] == UNASSIGNED) {
            return i;
        }
    }
    return 0;
}

int solverHeuristicVSIDS(SATSolver* solver) {
    int* positiveCount = (int*)calloc(solver->formula->numVars + 1, sizeof(int));
    int* negativeCount = (int*)calloc(solver->formula->numVars + 1, sizeof(int));
    
    // 统计每个变量在未满足子句中的出现次数
    for (int i = 0; i < solver->formula->clauses.size; i++) {
        if (solver->formula->clauses.data[i].satisfied) continue;
        
        for (int j = 0; j < solver->formula->clauses.data[i].literals.size; j++) {
            const Literal* lit = &solver->formula->clauses.data[i].literals.data[j];
            if (solver->assignment[lit->var] == UNASSIGNED) {
                if (lit->sign) {
                    positiveCount[lit->var]++;
                } else {
                    negativeCount[lit->var]++;
                }
            }
        }
    }
    
    // 选择出现次数最多的变量
    int maxCount = 0;
    int bestVar = 0;
    
    for (int i = 1; i <= solver->formula->numVars; i++) {
        if (solver->assignment[i] == UNASSIGNED) {
            int totalCount = positiveCount[i] + negativeCount[i];
            if (totalCount > maxCount) {
                maxCount = totalCount;
                bestVar = i;
            }
        }
    }
    
    free(positiveCount);
    free(negativeCount);
    
    return bestVar;
}

int solverHeuristicRandom(SATSolver* solver) {
    // 收集所有未赋值的变量
    int* unassigned = (int*)malloc(solver->formula->numVars * sizeof(int));
    int count = 0;
    
    for (int i = 1; i <= solver->formula->numVars; i++) {
        if (solver->assignment[i] == UNASSIGNED) {
            unassigned[count++] = i;
        }
    }
    
    int result = 0;
    if (count > 0) {
        result = unassigned[rand() % count];
    }
    
    free(unassigned);
    return result;
}

// ===== 赋值操作 =====
bool solverAssignVariable(SATSolver* solver, int var, bool value) {
    if (var < 1 || var > solver->formula->numVars) {
        return false;
    }
    
    solver->assignment[var] = value ? SAT_TRUE : SAT_FALSE;
    return true;
}

void solverUnassignVariable(SATSolver* solver, int var) {
    if (var >= 1 && var <= solver->formula->numVars) {
        solver->assignment[var] = UNASSIGNED;
    }
}

// ===== 公式状态检查 =====
bool solverIsFormulaSatisfied(const SATSolver* solver) {
    if (!solver || !solver->formula) return false;
    
    for (int i = 0; i < solver->formula->clauses.size; i++) {
        const Clause* clause = &solver->formula->clauses.data[i];
        bool clauseSatisfied = false;
        
        for (int j = 0; j < clause->literals.size; j++) {
            const Literal* lit = &clause->literals.data[j];
            
            if ((solver->assignment[lit->var] == SAT_TRUE && lit->sign) || 
                (solver->assignment[lit->var] == SAT_FALSE && !lit->sign)) {
                clauseSatisfied = true;
                break;
            }
        }
        
        if (!clauseSatisfied) {
            return false;
        }
    }
    return true;
}

bool solverIsFormulaUnsatisfied(const SATSolver* solver) {
    if (!solver || !solver->formula) return false;
    
    for (int i = 0; i < solver->formula->clauses.size; i++) {
        const Clause* clause = &solver->formula->clauses.data[i];
        if (clause->satisfied) continue;
        
        bool hasUnassigned = false;
        bool isSatisfied = false;
        
        for (int j = 0; j < clause->literals.size; j++) {
            const Literal* lit = &clause->literals.data[j];
            
            if (solver->assignment[lit->var] == UNASSIGNED) {
                hasUnassigned = true;
            } else if ((solver->assignment[lit->var] == SAT_TRUE && lit->sign) || 
                      (solver->assignment[lit->var] == SAT_FALSE && !lit->sign)) {
                isSatisfied = true;
                break;
            }
        }
        
        if (!isSatisfied && !hasUnassigned) {
            return true;
        }
    }
    return false;
}

// ===== 解验证和输出 =====
bool solverVerifySolution(const SATSolver* solver) {
    if (!solverIsFormulaSatisfied(solver)) {
        return false;
    }
    
    for (int i = 0; i < solver->formula->clauses.size; i++) {
        const Clause* clause = &solver->formula->clauses.data[i];
        bool clauseSatisfied = false;
        
        for (int j = 0; j < clause->literals.size; j++) {
            const Literal* lit = &clause->literals.data[j];
            
            if ((solver->assignment[lit->var] == SAT_TRUE && lit->sign) || 
                (solver->assignment[lit->var] == SAT_FALSE && !lit->sign)) {
                clauseSatisfied = true;
                break;
            }
        }
        
        if (!clauseSatisfied) {
            printf("验证失败：子句 %d 未被满足\n", i + 1);
            return false;
        }
    }
    
    printf("解验证成功！\n");
    return true;
}

void solverPrintSolution(const SATSolver* solver) {
    if (!solver || !solver->formula) return;
    
    printf("=== 求解结果 ===\n");
    for (int i = 1; i <= solver->formula->numVars; i++) {
        printf("x%d = ", i);
        if (solver->assignment[i] == SAT_TRUE) {
            printf("TRUE");
        } else if (solver->assignment[i] == SAT_FALSE) {
            printf("FALSE");
        } else {
            printf("UNASSIGNED");
        }
        printf("\n");
    }
    printf("===============\n");
}

void solverPrintStatistics(const SATSolver* solver) {
    if (!solver) return;
    
    printf("=== 性能统计 ===\n");
    printf("求解时间: %.3f 毫秒\n", solverGetSolvingTime(solver));
    printf("决策次数: %lld\n", solver->stats.decisions);
    printf("冲突次数: %lld\n", solver->stats.conflicts);
    printf("传播次数: %lld\n", solver->stats.propagations);
    printf("回溯次数: %lld\n", solver->stats.backtracks);
    printf("===============\n");
}

void solverSaveSolution(const SATSolver* solver, const char* filename) {
    if (!solver || !filename) return;
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "错误：无法创建输出文件 %s\n", filename);
        return;
    }
    
    fprintf(file, "c SAT求解结果\n");
    fprintf(file, "c 求解时间: %.3f 毫秒\n", solverGetSolvingTime(solver));
    fprintf(file, "c 决策次数: %lld\n", solver->stats.decisions);
    fprintf(file, "c 冲突次数: %lld\n", solver->stats.conflicts);
    fprintf(file, "c 传播次数: %lld\n", solver->stats.propagations);
    
    if (solverIsFormulaSatisfied(solver)) {
        fprintf(file, "s SATISFIABLE\n");
        fprintf(file, "v ");
        for (int i = 1; i <= solver->formula->numVars; i++) {
            if (solver->assignment[i] == SAT_FALSE) {
                fprintf(file, "-");
            }
            fprintf(file, "%d ", i);
        }
        fprintf(file, "0\n");
    } else {
        fprintf(file, "s UNSATISFIABLE\n");
    }
    
    fclose(file);
}

// ===== 性能分析 =====
double solverGetSolvingTime(const SATSolver* solver) {
    if (!solver) return 0.0;
    return statsGetSolvingTime(&solver->stats);
}

void solverStartTiming(SATSolver* solver) {
    if (solver) {
        solver->stats.startTime = clock();
    }
}

void solverStopTiming(SATSolver* solver) {
    if (solver) {
        solver->stats.endTime = clock();
    }
}