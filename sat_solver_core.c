#include "sat_solver_c.h"

// ===== SAT求解器创建和销毁 =====
SATSolver* satSolverCreate() {
    SATSolver* solver = (SATSolver*)malloc(sizeof(SATSolver));
    if (!solver) return NULL;
    
    solver->formula = createCNFFormula();
    solver->assignment = NULL;
    
    solver->decisionStack.data = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    solver->decisionStack.size = 0;
    solver->decisionStack.capacity = INITIAL_CAPACITY;
    
    solver->propagationQueue.data = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    solver->propagationQueue.size = 0;
    solver->propagationQueue.capacity = INITIAL_CAPACITY;
    
    solver->decisions = 0;
    solver->conflicts = 0;
    solver->propagations = 0;
    solver->useHeuristics = false;
    
    return solver;
}

void satSolverDestroy(SATSolver* solver) {
    if (solver) {
        destroyCNFFormula(&solver->formula);
        if (solver->assignment) {
            free(solver->assignment);
        }
        if (solver->decisionStack.data) {
            free(solver->decisionStack.data);
        }
        if (solver->propagationQueue.data) {
            free(solver->propagationQueue.data);
        }
        free(solver);
    }
}

// ===== CNF文件加载 =====
bool satSolverLoadCNF(SATSolver* solver, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "错误：无法打开文件 %s\n", filename);
        return false;
    }
    
    char line[MAX_LINE_LENGTH];
    bool foundHeader = false;
    
    while (fgets(line, sizeof(line), file)) {
        // 跳过注释和空行
        if (line[0] == 'c' || line[0] == '\n' || line[0] == '\0') {
            continue;
        }
        
        if (line[0] == 'p') {
            // 解析头部信息
            char p[10], cnf[10];
            if (sscanf(line, "%s %s %d %d", p, cnf, &solver->formula.numVars, &solver->formula.numClauses) != 4) {
                fprintf(stderr, "错误：CNF文件头部格式不正确\n");
                fclose(file);
                return false;
            }
            foundHeader = true;
            
            // 初始化赋值数组
            solver->assignment = (int*)malloc((solver->formula.numVars + 1) * sizeof(int));
            for (int i = 0; i <= solver->formula.numVars; i++) {
                solver->assignment[i] = UNASSIGNED;
            }
            continue;
        }
        
        if (!foundHeader) {
            fprintf(stderr, "错误：未找到CNF文件头部\n");
            fclose(file);
            return false;
        }
        
        // 解析子句
        Clause clause = createClause();
        char* token = strtok(line, " \t\n");
        
        while (token != NULL) {
            int lit = atoi(token);
            if (lit == 0) {
                break; // 子句结束
            }
            
            bool sign = (lit > 0);
            int var = abs(lit);
            
            if (var > solver->formula.numVars) {
                fprintf(stderr, "错误：变量编号超出范围: %d\n", var);
                destroyClause(&clause);
                fclose(file);
                return false;
            }
            
            clauseAddLiteral(&clause, createLiteral(var, sign));
            token = strtok(NULL, " \t\n");
        }
        
        if (!clauseIsEmpty(&clause)) {
            formulaAddClause(&solver->formula, clause);
        } else {
            destroyClause(&clause);
        }
    }
    
    fclose(file);
    
    // 验证子句数量
    if (solver->formula.clauses.size != solver->formula.numClauses) {
        printf("警告：实际子句数量 (%d) 与头部声明不符 (%d)\n", 
               solver->formula.clauses.size, solver->formula.numClauses);
        solver->formula.numClauses = solver->formula.clauses.size;
    }
    
    return true;
}

// ===== 访问器函数 =====
int satSolverGetNumVars(const SATSolver* solver) {
    return solver->formula.numVars;
}

int satSolverGetNumClauses(const SATSolver* solver) {
    return solver->formula.numClauses;
}

void satSolverPrintFormula(const SATSolver* solver) {
    printf("=== CNF公式内容 ===\n");
    formulaPrint(&solver->formula);
    printf("===================\n");
}

// ===== 赋值操作 =====
void satSolverAssignLiteral(SATSolver* solver, int var, bool value) {
    solver->assignment[var] = value ? TRUE_VAL : FALSE_VAL;
}

void satSolverUnassignVariable(SATSolver* solver, int var) {
    solver->assignment[var] = UNASSIGNED;
}

// ===== 公式状态检查 =====
bool satSolverIsFormulaSatisfied(const SATSolver* solver) {
    for (int i = 0; i < solver->formula.clauses.size; i++) {
        const Clause* clause = &solver->formula.clauses.data[i];
        bool clauseSatisfied = false;
        
        for (int j = 0; j < clause->literals.size; j++) {
            const Literal* lit = &clause->literals.data[j];
            
            if ((solver->assignment[lit->var] == TRUE_VAL && lit->sign) || 
                (solver->assignment[lit->var] == FALSE_VAL && !lit->sign)) {
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

bool satSolverIsFormulaUnsatisfied(const SATSolver* solver) {
    for (int i = 0; i < solver->formula.clauses.size; i++) {
        const Clause* clause = &solver->formula.clauses.data[i];
        if (clause->satisfied) continue;
        
        bool hasUnassigned = false;
        bool isSatisfied = false;
        
        for (int j = 0; j < clause->literals.size; j++) {
            const Literal* lit = &clause->literals.data[j];
            
            if (solver->assignment[lit->var] == UNASSIGNED) {
                hasUnassigned = true;
            } else if ((solver->assignment[lit->var] == TRUE_VAL && lit->sign) || 
                      (solver->assignment[lit->var] == FALSE_VAL && !lit->sign)) {
                isSatisfied = true;
                break;
            }
        }
        
        if (!isSatisfied && !hasUnassigned) {
            return true; // 找到不满足的子句
        }
    }
    return false;
}

// ===== 结果输出 =====
void satSolverPrintSolution(const SATSolver* solver) {
    printf("=== 求解结果 ===\n");
    for (int i = 1; i <= solver->formula.numVars; i++) {
        printf("x%d = ", i);
        if (solver->assignment[i] == TRUE_VAL) {
            printf("TRUE");
        } else if (solver->assignment[i] == FALSE_VAL) {
            printf("FALSE");
        } else {
            printf("UNASSIGNED");
        }
        printf("\n");
    }
    printf("===============\n");
}

void satSolverPrintStatistics(const SATSolver* solver) {
    printf("=== 性能统计 ===\n");
    printf("求解时间: %.3f 毫秒\n", satSolverGetSolvingTime(solver));
    printf("决策次数: %lld\n", solver->decisions);
    printf("冲突次数: %lld\n", solver->conflicts);
    printf("传播次数: %lld\n", solver->propagations);
    printf("===============\n");
}

double satSolverGetSolvingTime(const SATSolver* solver) {
    return ((double)(solver->endTime - solver->startTime) / CLOCKS_PER_SEC) * 1000.0;
}

void satSolverSaveSolution(const SATSolver* solver, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "错误：无法创建输出文件 %s\n", filename);
        return;
    }
    
    fprintf(file, "c SAT求解结果\n");
    fprintf(file, "c 求解时间: %.3f 毫秒\n", satSolverGetSolvingTime(solver));
    fprintf(file, "c 决策次数: %lld\n", solver->decisions);
    fprintf(file, "c 冲突次数: %lld\n", solver->conflicts);
    fprintf(file, "c 传播次数: %lld\n", solver->propagations);
    
    if (satSolverIsFormulaSatisfied(solver)) {
        fprintf(file, "s SATISFIABLE\n");
        fprintf(file, "v ");
        for (int i = 1; i <= solver->formula.numVars; i++) {
            if (solver->assignment[i] == FALSE_VAL) {
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

bool satSolverVerifySolution(const SATSolver* solver) {
    if (!satSolverIsFormulaSatisfied(solver)) {
        return false;
    }
    
    // 验证每个子句都被满足
    for (int i = 0; i < solver->formula.clauses.size; i++) {
        const Clause* clause = &solver->formula.clauses.data[i];
        bool clauseSatisfied = false;
        
        for (int j = 0; j < clause->literals.size; j++) {
            const Literal* lit = &clause->literals.data[j];
            
            if ((solver->assignment[lit->var] == TRUE_VAL && lit->sign) || 
                (solver->assignment[lit->var] == FALSE_VAL && !lit->sign)) {
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