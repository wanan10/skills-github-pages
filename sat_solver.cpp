#include "sat_solver.h"

// SATSolver实现
SATSolver::SATSolver() : assignment(nullptr), decisions(0), conflicts(0), propagations(0), useHeuristics(false) {}

SATSolver::~SATSolver() {
    if (assignment) {
        delete[] assignment;
    }
}

bool SATSolver::loadCNF(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "错误：无法打开文件 " << filename << std::endl;
        return false;
    }
    
    std::string line;
    bool foundHeader = false;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == 'c') {
            continue; // 跳过注释和空行
        }
        
        if (line[0] == 'p') {
            // 解析头部信息
            char p, cnf[10];
            if (sscanf(line.c_str(), "%c %s %d %d", &p, cnf, &formula.numVars, &formula.numClauses) != 4) {
                std::cerr << "错误：CNF文件头部格式不正确" << std::endl;
                return false;
            }
            foundHeader = true;
            
            // 初始化赋值数组
            assignment = new int[formula.numVars + 1];
            for (int i = 0; i <= formula.numVars; i++) {
                assignment[i] = UNASSIGNED;
            }
            continue;
        }
        
        if (!foundHeader) {
            std::cerr << "错误：未找到CNF文件头部" << std::endl;
            return false;
        }
        
        // 解析子句
        Clause clause;
        
        // 创建一个可修改的字符串副本
        char* lineBuffer = new char[line.length() + 1];
        strcpy(lineBuffer, line.c_str());
        
        char* token = strtok(lineBuffer, " \t");
        
        while (token != nullptr) {
            int lit = atoi(token);
            if (lit == 0) {
                break; // 子句结束
            }
            
            bool sign = (lit > 0);
            int var = abs(lit);
            
            if (var > formula.numVars) {
                std::cerr << "错误：变量编号超出范围: " << var << std::endl;
                delete[] lineBuffer;
                return false;
            }
            
            clause.addLiteral(Literal(var, sign));
            token = strtok(nullptr, " \t");
        }
        
        delete[] lineBuffer;
        
        if (!clause.isEmpty()) {
            formula.addClause(clause);
        }
    }
    
    file.close();
    
    if (formula.clauses.getSize() != formula.numClauses) {
        std::cout << "警告：实际子句数量 (" << formula.clauses.getSize() 
                  << ") 与头部声明不符 (" << formula.numClauses << ")" << std::endl;
        formula.numClauses = formula.clauses.getSize();
    }
    
    return true;
}

void SATSolver::printFormula() const {
    std::cout << "=== CNF公式内容 ===" << std::endl;
    formula.print();
    std::cout << "===================" << std::endl;
}

bool SATSolver::unitPropagate() {
    bool propagated = false;
    
    do {
        propagated = false;
        
        for (int i = 0; i < formula.clauses.getSize(); i++) {
            Clause& clause = formula.clauses[i];
            if (clause.satisfied) continue;
            
            int unassignedCount = 0;
            int unassignedVar = 0;
            bool unassignedSign = true;
            bool clauseSatisfied = false;
            
            // 检查子句状态
            for (int j = 0; j < clause.literals.getSize(); j++) {
                const Literal& lit = clause.literals[j];
                
                if (assignment[lit.var] == UNASSIGNED) {
                    unassignedCount++;
                    unassignedVar = lit.var;
                    unassignedSign = lit.sign;
                } else if ((assignment[lit.var] == TRUE && lit.sign) || 
                          (assignment[lit.var] == FALSE && !lit.sign)) {
                    clauseSatisfied = true;
                    clause.satisfied = true;
                    break;
                }
            }
            
            if (clauseSatisfied) continue;
            
            if (unassignedCount == 0) {
                // 子句不满足，产生冲突
                return false;
            } else if (unassignedCount == 1) {
                // 单元子句，进行传播
                assignLiteral(unassignedVar, unassignedSign);
                propagationQueue.push_back(unassignedVar);
                propagated = true;
                propagations++;
            }
        }
    } while (propagated);
    
    return true;
}

int SATSolver::selectBranchingVariable() {
    if (useHeuristics) {
        return selectBranchingVariableWithHeuristic();
    }
    
    // 简单策略：选择第一个未赋值的变量
    for (int i = 1; i <= formula.numVars; i++) {
        if (assignment[i] == UNASSIGNED) {
            return i;
        }
    }
    return 0;
}

int SATSolver::selectBranchingVariableWithHeuristic() {
    // VSIDS启发式：选择在最多子句中出现的变量
    int* positiveCount = new int[formula.numVars + 1];
    int* negativeCount = new int[formula.numVars + 1];
    
    for (int i = 0; i <= formula.numVars; i++) {
        positiveCount[i] = 0;
        negativeCount[i] = 0;
    }
    
    // 统计每个变量在未满足子句中的出现次数
    for (int i = 0; i < formula.clauses.getSize(); i++) {
        if (formula.clauses[i].satisfied) continue;
        
        for (int j = 0; j < formula.clauses[i].literals.getSize(); j++) {
            const Literal& lit = formula.clauses[i].literals[j];
            if (assignment[lit.var] == UNASSIGNED) {
                if (lit.sign) {
                    positiveCount[lit.var]++;
                } else {
                    negativeCount[lit.var]++;
                }
            }
        }
    }
    
    // 选择出现次数最多的变量
    int maxCount = 0;
    int bestVar = 0;
    
    for (int i = 1; i <= formula.numVars; i++) {
        if (assignment[i] == UNASSIGNED) {
            int totalCount = positiveCount[i] + negativeCount[i];
            if (totalCount > maxCount) {
                maxCount = totalCount;
                bestVar = i;
            }
        }
    }
    
    delete[] positiveCount;
    delete[] negativeCount;
    
    return bestVar;
}

bool SATSolver::isFormulaUnsatisfied() {
    for (int i = 0; i < formula.clauses.getSize(); i++) {
        const Clause& clause = formula.clauses[i];
        if (clause.satisfied) continue;
        
        bool hasUnassigned = false;
        bool isSatisfied = false;
        
        for (int j = 0; j < clause.literals.getSize(); j++) {
            const Literal& lit = clause.literals[j];
            
            if (assignment[lit.var] == UNASSIGNED) {
                hasUnassigned = true;
            } else if ((assignment[lit.var] == TRUE && lit.sign) || 
                      (assignment[lit.var] == FALSE && !lit.sign)) {
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

bool SATSolver::isFormulaSatisfied() const {
    for (int i = 0; i < formula.clauses.getSize(); i++) {
        const Clause& clause = formula.clauses[i];
        bool clauseSatisfied = false;
        
        for (int j = 0; j < clause.literals.getSize(); j++) {
            const Literal& lit = clause.literals[j];
            
            if ((assignment[lit.var] == TRUE && lit.sign) || 
                (assignment[lit.var] == FALSE && !lit.sign)) {
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

void SATSolver::assignLiteral(int var, bool value) {
    assignment[var] = value ? TRUE : FALSE;
}

void SATSolver::unassignVariable(int var) {
    assignment[var] = UNASSIGNED;
}

void SATSolver::backtrack() {
    // 简化的回溯：撤销最后的决策
    if (!decisionStack.empty()) {
        int var = decisionStack[decisionStack.getSize() - 1];
        decisionStack.pop_back();
        unassignVariable(var);
        
        // 重置子句满足状态
        for (int i = 0; i < formula.clauses.getSize(); i++) {
            formula.clauses[i].satisfied = false;
        }
    }
}

bool SATSolver::dpll() {
    // 单元传播
    if (!unitPropagate()) {
        conflicts++;
        return false;
    }
    
    // 检查是否所有子句都满足
    if (isFormulaSatisfied()) {
        return true;
    }
    
    // 选择分支变量
    int var = selectBranchingVariable();
    if (var == 0) {
        return isFormulaSatisfied();
    }
    
    decisions++;
    
    // 尝试赋值为true
    decisionStack.push_back(var);
    assignLiteral(var, true);
    
    if (dpll()) {
        return true;
    }
    
    // 回溯并尝试赋值为false
    unassignVariable(var);
    assignLiteral(var, false);
    
    if (dpll()) {
        return true;
    }
    
    // 两种赋值都失败，回溯
    unassignVariable(var);
    decisionStack.pop_back();
    return false;
}

bool SATSolver::solve(bool useOptimization) {
    useHeuristics = useOptimization;
    
    // 重置统计信息
    decisions = 0;
    conflicts = 0;
    propagations = 0;
    
    // 重置赋值
    for (int i = 1; i <= formula.numVars; i++) {
        assignment[i] = UNASSIGNED;
    }
    
    // 重置子句状态
    for (int i = 0; i < formula.clauses.getSize(); i++) {
        formula.clauses[i].satisfied = false;
    }
    
    decisionStack.clear();
    propagationQueue.clear();
    
    startTime = clock();
    bool result = dpll();
    endTime = clock();
    
    return result;
}

void SATSolver::printSolution() const {
    std::cout << "=== 求解结果 ===" << std::endl;
    for (int i = 1; i <= formula.numVars; i++) {
        std::cout << "x" << i << " = ";
        if (assignment[i] == TRUE) {
            std::cout << "TRUE";
        } else if (assignment[i] == FALSE) {
            std::cout << "FALSE";
        } else {
            std::cout << "UNASSIGNED";
        }
        std::cout << std::endl;
    }
    std::cout << "===============" << std::endl;
}

void SATSolver::printStatistics() const {
    std::cout << "=== 性能统计 ===" << std::endl;
    std::cout << "求解时间: " << getSolvingTime() << " 毫秒" << std::endl;
    std::cout << "决策次数: " << decisions << std::endl;
    std::cout << "冲突次数: " << conflicts << std::endl;
    std::cout << "传播次数: " << propagations << std::endl;
    std::cout << "===============" << std::endl;
}

double SATSolver::getSolvingTime() const {
    return ((double)(endTime - startTime) / CLOCKS_PER_SEC) * 1000.0;
}

void SATSolver::saveSolution(const char* filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "错误：无法创建输出文件 " << filename << std::endl;
        return;
    }
    
    file << "c SAT求解结果" << std::endl;
    file << "c 求解时间: " << getSolvingTime() << " 毫秒" << std::endl;
    file << "c 决策次数: " << decisions << std::endl;
    file << "c 冲突次数: " << conflicts << std::endl;
    file << "c 传播次数: " << propagations << std::endl;
    
    if (isFormulaSatisfied()) {
        file << "s SATISFIABLE" << std::endl;
        file << "v ";
        for (int i = 1; i <= formula.numVars; i++) {
            if (assignment[i] == FALSE) {
                file << "-";
            }
            file << i << " ";
        }
        file << "0" << std::endl;
    } else {
        file << "s UNSATISFIABLE" << std::endl;
    }
    
    file.close();
}

bool SATSolver::verifySolution() const {
    if (!isFormulaSatisfied()) {
        return false;
    }
    
    // 验证每个子句都被满足
    for (int i = 0; i < formula.clauses.getSize(); i++) {
        const Clause& clause = formula.clauses[i];
        bool clauseSatisfied = false;
        
        for (int j = 0; j < clause.literals.getSize(); j++) {
            const Literal& lit = clause.literals[j];
            
            if ((assignment[lit.var] == TRUE && lit.sign) || 
                (assignment[lit.var] == FALSE && !lit.sign)) {
                clauseSatisfied = true;
                break;
            }
        }
        
        if (!clauseSatisfied) {
            std::cout << "验证失败：子句 " << i + 1 << " 未被满足" << std::endl;
            return false;
        }
    }
    
    std::cout << "解验证成功！" << std::endl;
    return true;
}