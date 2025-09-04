#include "sat_solver_c.h"

// ===== 单元传播 =====
bool satSolverUnitPropagate(SATSolver* solver) {
    bool propagated = false;
    
    do {
        propagated = false;
        
        for (int i = 0; i < solver->formula.clauses.size; i++) {
            Clause* clause = &solver->formula.clauses.data[i];
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
                } else if ((solver->assignment[lit->var] == TRUE_VAL && lit->sign) || 
                          (solver->assignment[lit->var] == FALSE_VAL && !lit->sign)) {
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
                satSolverAssignLiteral(solver, unassignedVar, unassignedSign);
                intArrayPush(&solver->propagationQueue, unassignedVar);
                propagated = true;
                solver->propagations++;
            }
        }
    } while (propagated);
    
    return true;
}

// ===== 变量选择策略 =====
int satSolverSelectBranchingVariable(SATSolver* solver) {
    if (solver->useHeuristics) {
        return satSolverSelectBranchingVariableWithHeuristic(solver);
    }
    
    // 简单策略：选择第一个未赋值的变量
    for (int i = 1; i <= solver->formula.numVars; i++) {
        if (solver->assignment[i] == UNASSIGNED) {
            return i;
        }
    }
    return 0;
}

int satSolverSelectBranchingVariableWithHeuristic(SATSolver* solver) {
    // VSIDS启发式：选择在最多子句中出现的变量
    int* positiveCount = (int*)calloc(solver->formula.numVars + 1, sizeof(int));
    int* negativeCount = (int*)calloc(solver->formula.numVars + 1, sizeof(int));
    
    // 统计每个变量在未满足子句中的出现次数
    for (int i = 0; i < solver->formula.clauses.size; i++) {
        if (solver->formula.clauses.data[i].satisfied) continue;
        
        for (int j = 0; j < solver->formula.clauses.data[i].literals.size; j++) {
            const Literal* lit = &solver->formula.clauses.data[i].literals.data[j];
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
    
    for (int i = 1; i <= solver->formula.numVars; i++) {
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

// ===== DPLL主算法 =====
bool satSolverDPLL(SATSolver* solver) {
    // 单元传播
    if (!satSolverUnitPropagate(solver)) {
        solver->conflicts++;
        return false;
    }
    
    // 检查是否所有子句都满足
    if (satSolverIsFormulaSatisfied(solver)) {
        return true;
    }
    
    // 选择分支变量
    int var = satSolverSelectBranchingVariable(solver);
    if (var == 0) {
        return satSolverIsFormulaSatisfied(solver);
    }
    
    solver->decisions++;
    
    // 尝试赋值为true
    intArrayPush(&solver->decisionStack, var);
    satSolverAssignLiteral(solver, var, true);
    
    if (satSolverDPLL(solver)) {
        return true;
    }
    
    // 回溯并尝试赋值为false
    satSolverUnassignVariable(solver, var);
    satSolverAssignLiteral(solver, var, false);
    
    if (satSolverDPLL(solver)) {
        return true;
    }
    
    // 两种赋值都失败，回溯
    satSolverUnassignVariable(solver, var);
    intArrayPop(&solver->decisionStack);
    
    // 重置子句满足状态
    for (int i = 0; i < solver->formula.clauses.size; i++) {
        solver->formula.clauses.data[i].satisfied = false;
    }
    
    return false;
}

// ===== 求解主函数 =====
bool satSolverSolve(SATSolver* solver, bool useOptimization) {
    solver->useHeuristics = useOptimization;
    
    // 重置统计信息
    solver->decisions = 0;
    solver->conflicts = 0;
    solver->propagations = 0;
    
    // 重置赋值
    for (int i = 1; i <= solver->formula.numVars; i++) {
        solver->assignment[i] = UNASSIGNED;
    }
    
    // 重置子句状态
    for (int i = 0; i < solver->formula.clauses.size; i++) {
        solver->formula.clauses.data[i].satisfied = false;
    }
    
    intArrayClear(&solver->decisionStack);
    intArrayClear(&solver->propagationQueue);
    
    solver->startTime = clock();
    bool result = satSolverDPLL(solver);
    solver->endTime = clock();
    
    return result;
}