#include "sat_solver.h"
#include <iostream>
#include <cstring>

void printUsage() {
    std::cout << "SAT求解器 - 基于DPLL算法" << std::endl;
    std::cout << "用法：" << std::endl;
    std::cout << "  ./sat_solver -f <cnf_file> [-o] [-v] [-s <output_file>]" << std::endl;
    std::cout << "  ./sat_solver -sudoku" << std::endl;
    std::cout << std::endl;
    std::cout << "选项：" << std::endl;
    std::cout << "  -f <file>     指定CNF输入文件" << std::endl;
    std::cout << "  -o            使用优化策略（变量选择启发式）" << std::endl;
    std::cout << "  -v            验证解的正确性" << std::endl;
    std::cout << "  -s <file>     保存结果到文件" << std::endl;
    std::cout << "  -sudoku       启动数独游戏模式" << std::endl;
    std::cout << "  -h, --help    显示帮助信息" << std::endl;
}

void runPerformanceComparison(const char* filename) {
    std::cout << "=== 性能对比测试 ===" << std::endl;
    
    SATSolver solver1, solver2;
    
    // 加载同一个文件两次
    if (!solver1.loadCNF(filename) || !solver2.loadCNF(filename)) {
        std::cerr << "加载CNF文件失败" << std::endl;
        return;
    }
    
    std::cout << "测试文件: " << filename << std::endl;
    std::cout << "变量数: " << solver1.getNumVars() << std::endl;
    std::cout << "子句数: " << solver1.getNumClauses() << std::endl;
    std::cout << std::endl;
    
    // 基础DPLL求解
    std::cout << "基础DPLL求解..." << std::endl;
    bool result1 = solver1.solve(false);
    double time1 = solver1.getSolvingTime();
    
    std::cout << "结果: " << (result1 ? "SAT" : "UNSAT") << std::endl;
    std::cout << "时间: " << time1 << " 毫秒" << std::endl;
    solver1.printStatistics();
    std::cout << std::endl;
    
    // 优化DPLL求解
    std::cout << "优化DPLL求解..." << std::endl;
    bool result2 = solver2.solve(true);
    double time2 = solver2.getSolvingTime();
    
    std::cout << "结果: " << (result2 ? "SAT" : "UNSAT") << std::endl;
    std::cout << "时间: " << time2 << " 毫秒" << std::endl;
    solver2.printStatistics();
    std::cout << std::endl;
    
    // 计算优化率
    if (time1 > 0) {
        double optimizationRate = ((time1 - time2) / time1) * 100.0;
        std::cout << "=== 优化效果 ===" << std::endl;
        std::cout << "基础时间: " << time1 << " 毫秒" << std::endl;
        std::cout << "优化时间: " << time2 << " 毫秒" << std::endl;
        std::cout << "优化率: " << optimizationRate << "%" << std::endl;
        
        if (optimizationRate > 0) {
            std::cout << "优化效果: 提升 " << optimizationRate << "%" << std::endl;
        } else {
            std::cout << "优化效果: 下降 " << (-optimizationRate) << "%" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    // 解析命令行参数
    char* inputFile = nullptr;
    char* outputFile = nullptr;
    bool useOptimization = false;
    bool verify = false;
    bool sudokuMode = false;
    bool showComparison = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            inputFile = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0) {
            useOptimization = true;
        } else if (strcmp(argv[i], "-v") == 0) {
            verify = true;
        } else if (strcmp(argv[i], "-sudoku") == 0) {
            sudokuMode = true;
        } else if (strcmp(argv[i], "-c") == 0) {
            showComparison = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage();
            return 0;
        }
    }
    
    if (sudokuMode) {
        // 数独模式
        SudokuSolver sudokuSolver;
        sudokuSolver.playInteractiveGame();
        return 0;
    }
    
    if (!inputFile) {
        std::cerr << "错误：必须指定输入文件" << std::endl;
        printUsage();
        return 1;
    }
    
    if (showComparison) {
        runPerformanceComparison(inputFile);
        return 0;
    }
    
    // SAT求解模式
    SATSolver solver;
    
    std::cout << "加载CNF文件: " << inputFile << std::endl;
    if (!solver.loadCNF(inputFile)) {
        return 1;
    }
    
    std::cout << "变量数: " << solver.getNumVars() << std::endl;
    std::cout << "子句数: " << solver.getNumClauses() << std::endl;
    std::cout << std::endl;
    
    // 显示公式内容（用于验证解析正确性）
    std::cout << "是否显示公式内容以验证解析正确性？(y/n): ";
    char choice;
    std::cin >> choice;
    if (choice == 'y' || choice == 'Y') {
        solver.printFormula();
    }
    
    std::cout << std::endl << "开始求解..." << std::endl;
    if (useOptimization) {
        std::cout << "使用优化策略" << std::endl;
    }
    
    bool result = solver.solve(useOptimization);
    
    std::cout << std::endl << "=== 求解完成 ===" << std::endl;
    if (result) {
        std::cout << "结果: SATISFIABLE" << std::endl;
        solver.printSolution();
        
        if (verify) {
            std::cout << std::endl << "验证解的正确性..." << std::endl;
            solver.verifySolution();
        }
    } else {
        std::cout << "结果: UNSATISFIABLE" << std::endl;
    }
    
    std::cout << std::endl;
    solver.printStatistics();
    
    if (outputFile) {
        solver.saveSolution(outputFile);
        std::cout << "结果已保存到: " << outputFile << std::endl;
    }
    
    return 0;
}