#include "sat_solver_c.h"

void printUsage() {
    printf("SAT求解器 - 基于DPLL算法 (C语言版本)\n");
    printf("用法：\n");
    printf("  ./sat_solver_c -f <cnf_file> [-o] [-v] [-s <output_file>]\n");
    printf("  ./sat_solver_c -sudoku\n");
    printf("\n");
    printf("选项：\n");
    printf("  -f <file>     指定CNF输入文件\n");
    printf("  -o            使用优化策略（变量选择启发式）\n");
    printf("  -v            验证解的正确性\n");
    printf("  -s <file>     保存结果到文件\n");
    printf("  -sudoku       启动数独游戏模式\n");
    printf("  -h, --help    显示帮助信息\n");
}

void runPerformanceComparison(const char* filename) {
    printf("=== 性能对比测试 ===\n");
    
    SATSolver* solver1 = satSolverCreate();
    SATSolver* solver2 = satSolverCreate();
    
    if (!solver1 || !solver2) {
        fprintf(stderr, "内存分配失败\n");
        if (solver1) satSolverDestroy(solver1);
        if (solver2) satSolverDestroy(solver2);
        return;
    }
    
    // 加载同一个文件两次
    if (!satSolverLoadCNF(solver1, filename) || !satSolverLoadCNF(solver2, filename)) {
        fprintf(stderr, "加载CNF文件失败\n");
        satSolverDestroy(solver1);
        satSolverDestroy(solver2);
        return;
    }
    
    printf("测试文件: %s\n", filename);
    printf("变量数: %d\n", satSolverGetNumVars(solver1));
    printf("子句数: %d\n", satSolverGetNumClauses(solver1));
    printf("\n");
    
    // 基础DPLL求解
    printf("基础DPLL求解...\n");
    bool result1 = satSolverSolve(solver1, false);
    double time1 = satSolverGetSolvingTime(solver1);
    
    printf("结果: %s\n", result1 ? "SAT" : "UNSAT");
    printf("时间: %.3f 毫秒\n", time1);
    satSolverPrintStatistics(solver1);
    printf("\n");
    
    // 优化DPLL求解
    printf("优化DPLL求解...\n");
    bool result2 = satSolverSolve(solver2, true);
    double time2 = satSolverGetSolvingTime(solver2);
    
    printf("结果: %s\n", result2 ? "SAT" : "UNSAT");
    printf("时间: %.3f 毫秒\n", time2);
    satSolverPrintStatistics(solver2);
    printf("\n");
    
    // 计算优化率
    if (time1 > 0) {
        double optimizationRate = ((time1 - time2) / time1) * 100.0;
        printf("=== 优化效果 ===\n");
        printf("基础时间: %.3f 毫秒\n", time1);
        printf("优化时间: %.3f 毫秒\n", time2);
        printf("优化率: %.1f%%\n", optimizationRate);
        
        if (optimizationRate > 0) {
            printf("优化效果: 提升 %.1f%%\n", optimizationRate);
        } else {
            printf("优化效果: 下降 %.1f%%\n", -optimizationRate);
        }
    }
    
    satSolverDestroy(solver1);
    satSolverDestroy(solver2);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }
    
    // 解析命令行参数
    char* inputFile = NULL;
    char* outputFile = NULL;
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
        SudokuSolver* sudokuSolver = sudokuSolverCreate();
        if (!sudokuSolver) {
            fprintf(stderr, "内存分配失败\n");
            return 1;
        }
        sudokuSolverPlayInteractiveGame(sudokuSolver);
        sudokuSolverDestroy(sudokuSolver);
        return 0;
    }
    
    if (!inputFile) {
        fprintf(stderr, "错误：必须指定输入文件\n");
        printUsage();
        return 1;
    }
    
    if (showComparison) {
        runPerformanceComparison(inputFile);
        return 0;
    }
    
    // SAT求解模式
    SATSolver* solver = satSolverCreate();
    if (!solver) {
        fprintf(stderr, "内存分配失败\n");
        return 1;
    }
    
    printf("加载CNF文件: %s\n", inputFile);
    if (!satSolverLoadCNF(solver, inputFile)) {
        satSolverDestroy(solver);
        return 1;
    }
    
    printf("变量数: %d\n", satSolverGetNumVars(solver));
    printf("子句数: %d\n", satSolverGetNumClauses(solver));
    printf("\n");
    
    // 显示公式内容（用于验证解析正确性）
    printf("是否显示公式内容以验证解析正确性？(y/n): ");
    char choice;
    if (scanf(" %c", &choice) == 1 && (choice == 'y' || choice == 'Y')) {
        satSolverPrintFormula(solver);
    }
    
    printf("\n开始求解...\n");
    if (useOptimization) {
        printf("使用优化策略\n");
    }
    
    bool result = satSolverSolve(solver, useOptimization);
    
    printf("\n=== 求解完成 ===\n");
    if (result) {
        printf("结果: SATISFIABLE\n");
        satSolverPrintSolution(solver);
        
        if (verify) {
            printf("\n验证解的正确性...\n");
            satSolverVerifySolution(solver);
        }
    } else {
        printf("结果: UNSATISFIABLE\n");
    }
    
    printf("\n");
    satSolverPrintStatistics(solver);
    
    if (outputFile) {
        satSolverSaveSolution(solver, outputFile);
        printf("结果已保存到: %s\n", outputFile);
    }
    
    satSolverDestroy(solver);
    return 0;
}