#include "display.h"
#include <stdarg.h>

// 为strdup函数提供声明
char* strdup(const char* s);

// ===== 主控制器创建和销毁 =====
MainController* displayCreate() {
    MainController* controller = (MainController*)malloc(sizeof(MainController));
    if (!controller) return NULL;
    
    // 初始化配置
    memset(&controller->config, 0, sizeof(ProgramConfig));
    controller->config.heuristic = HEURISTIC_VSIDS;
    controller->config.mode = MODE_INTERACTIVE;
    controller->config.showStatistics = true;
    
    // 创建组件
    controller->parser = cnfParserCreate();
    controller->satSolver = solverCreate();
    controller->sudokuSolver = xsudokuCreate();
    
    if (!controller->parser || !controller->satSolver || !controller->sudokuSolver) {
        displayDestroy(controller);
        return NULL;
    }
    
    return controller;
}

void displayDestroy(MainController* controller) {
    if (controller) {
        if (controller->config.inputFile) free(controller->config.inputFile);
        if (controller->config.outputFile) free(controller->config.outputFile);
        if (controller->parser) cnfParserDestroy(controller->parser);
        if (controller->satSolver) solverDestroy(controller->satSolver);
        if (controller->sudokuSolver) xsudokuDestroy(controller->sudokuSolver);
        free(controller);
    }
}

// ===== 主运行函数 =====
int displayRun(MainController* controller, int argc, char* argv[]) {
    if (!controller) return 1;
    
    displayPrintHeader("SAT求解器 - 基于DPLL算法");
    
    // 解析命令行参数
    if (!displayParseCommandLine(controller, argc, argv)) {
        displayPrintUsage(argv[0]);
        return 1;
    }
    
    // 根据模式执行相应功能
    switch (controller->config.mode) {
        case MODE_SAT_SOLVE:
            return displaySATSolveMode(controller);
        case MODE_SUDOKU_GAME:
            return displaySudokuGameMode(controller);
        case MODE_PERFORMANCE_TEST:
            return displayPerformanceTestMode(controller);
        case MODE_BATCH_SOLVE:
            return displayBatchSolveMode(controller);
        case MODE_INTERACTIVE:
        default:
            return displayInteractiveMode(controller);
    }
}

// ===== 命令行解析 =====
bool displayParseCommandLine(MainController* controller, int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            controller->config.inputFile = strdup(argv[++i]);
            controller->config.mode = MODE_SAT_SOLVE;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            controller->config.outputFile = strdup(argv[++i]);
        } else if (strcmp(argv[i], "--optimize") == 0) {
            controller->config.useOptimization = true;
        } else if (strcmp(argv[i], "--verify") == 0) {
            controller->config.verifyResult = true;
        } else if (strcmp(argv[i], "--show-formula") == 0) {
            controller->config.showFormula = true;
        } else if (strcmp(argv[i], "--quiet") == 0) {
            controller->config.quietMode = true;
        } else if (strcmp(argv[i], "--sudoku") == 0) {
            controller->config.mode = MODE_SUDOKU_GAME;
        } else if (strcmp(argv[i], "--performance") == 0) {
            controller->config.mode = MODE_PERFORMANCE_TEST;
        } else if (strcmp(argv[i], "--batch") == 0) {
            controller->config.mode = MODE_BATCH_SOLVE;
        } else if (strcmp(argv[i], "--interactive") == 0) {
            controller->config.mode = MODE_INTERACTIVE;
        } else if (strcmp(argv[i], "--heuristic") == 0 && i + 1 < argc) {
            char* heuristic = argv[++i];
            if (strcmp(heuristic, "none") == 0) {
                controller->config.heuristic = HEURISTIC_NONE;
            } else if (strcmp(heuristic, "first") == 0) {
                controller->config.heuristic = HEURISTIC_FIRST_UNASSIGNED;
            } else if (strcmp(heuristic, "vsids") == 0) {
                controller->config.heuristic = HEURISTIC_VSIDS;
            } else if (strcmp(heuristic, "random") == 0) {
                controller->config.heuristic = HEURISTIC_RANDOM;
            }
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            displayPrintUsage(argv[0]);
            return false;
        } else if (strcmp(argv[i], "--version") == 0) {
            displayPrintVersion();
            return false;
        }
    }
    
    return true;
}

void displayPrintUsage(const char* programName) {
    printf("用法: %s [选项]\n\n", programName);
    printf("选项:\n");
    printf("  -f <文件>          指定CNF输入文件\n");
    printf("  -o <文件>          指定输出文件\n");
    printf("  --optimize         使用优化策略\n");
    printf("  --verify           验证求解结果\n");
    printf("  --show-formula     显示CNF公式\n");
    printf("  --quiet            静默模式\n");
    printf("  --sudoku           数独游戏模式\n");
    printf("  --performance      性能测试模式\n");
    printf("  --batch            批量求解模式\n");
    printf("  --interactive      交互模式（默认）\n");
    printf("  --heuristic <类型> 启发式策略 (none|first|vsids|random)\n");
    printf("  --help, -h         显示帮助信息\n");
    printf("  --version          显示版本信息\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s -f test.cnf --optimize --verify\n", programName);
    printf("  %s --sudoku\n", programName);
    printf("  %s --performance\n", programName);
}

void displayPrintVersion() {
    printf("SAT求解器 v1.0\n");
    printf("基于DPLL算法的布尔可满足性问题求解器\n");
    printf("支持CNF格式和X数独求解\n");
    printf("编译时间: %s %s\n", __DATE__, __TIME__);
}

// ===== 主要功能模块 =====
int displaySATSolveMode(MainController* controller) {
    if (!controller->config.inputFile) {
        displayError("SAT求解模式需要指定输入文件");
        return 1;
    }
    
    displayInfo("加载CNF文件: %s", controller->config.inputFile);
    
    // 加载CNF文件
    if (!displayLoadCNFFile(controller, controller->config.inputFile)) {
        return 1;
    }
    
    // 显示公式信息
    if (!controller->config.quietMode) {
        printf("变量数: %d\n", controller->satSolver->formula->numVars);
        printf("子句数: %d\n", controller->satSolver->formula->numClauses);
    }
    
    // 显示公式内容
    if (controller->config.showFormula) {
        displayPrintFormula(controller->satSolver->formula, true);
    }
    
    // 设置启发式
    solverSetHeuristic(controller->satSolver, controller->config.heuristic);
    
    // 求解
    displayInfo("开始求解...");
    if (controller->config.useOptimization) {
        displayInfo("使用优化策略");
    }
    
    SolverResult result = solverSolve(controller->satSolver);
    
    // 显示结果
    displaySATResult(controller->satSolver, result);
    
    // 验证结果
    if (controller->config.verifyResult && result == SOLVER_SAT) {
        displayInfo("验证解的正确性...");
        if (solverVerifySolution(controller->satSolver)) {
            displaySuccess("解验证成功！");
        } else {
            displayError("解验证失败！");
        }
    }
    
    // 保存结果
    if (controller->config.outputFile) {
        solverSaveSolution(controller->satSolver, controller->config.outputFile);
        displayInfo("结果已保存到: %s", controller->config.outputFile);
    }
    
    return (result == SOLVER_SAT) ? 0 : 1;
}

int displaySudokuGameMode(MainController* controller) {
    displayPrintHeader("X数独游戏模式");
    xsudokuPlayGame(controller->sudokuSolver);
    return 0;
}

int displayPerformanceTestMode(MainController* controller) {
    displayPrintHeader("性能测试模式");
    
    if (!controller->config.inputFile) {
        displayError("性能测试需要指定输入文件");
        return 1;
    }
    
    displayPerformanceComparison(controller->config.inputFile);
    return 0;
}

int displayBatchSolveMode(MainController* controller) {
    displayPrintHeader("批量求解模式");
    displayInfo("批量求解功能开发中...");
    return 0;
}

int displayInteractiveMode(MainController* controller) {
    displayPrintHeader("交互模式");
    
    while (true) {
        displayShowMainMenu();
        int choice = displayGetUserChoice("请选择功能", 1, 5);
        
        switch (choice) {
            case 1:
                displaySATSolveMode(controller);
                break;
            case 2:
                displaySudokuGameMode(controller);
                break;
            case 3:
                displayPerformanceTestMode(controller);
                break;
            case 4:
                printf("批量求解功能开发中...\n");
                break;
            case 5:
                displayInfo("感谢使用SAT求解器！");
                return 0;
            default:
                displayWarning("无效选择");
                break;
        }
        
        if (!displayConfirm("是否继续使用？")) {
            break;
        }
    }
    
    return 0;
}

// ===== 交互界面 =====
void displayShowMainMenu() {
    displayPrintSeparator('=', 50);
    printf("           SAT求解器主菜单\n");
    displayPrintSeparator('=', 50);
    printf("1. SAT问题求解\n");
    printf("2. X数独游戏\n");
    printf("3. 性能测试\n");
    printf("4. 批量求解\n");
    printf("5. 退出程序\n");
    displayPrintSeparator('-', 50);
}

int displayGetUserChoice(const char* prompt, int minChoice, int maxChoice) {
    int choice;
    do {
        printf("%s (%d-%d): ", prompt, minChoice, maxChoice);
        if (scanf("%d", &choice) != 1) {
            choice = -1;
            while (getchar() != '\n'); // 清空输入缓冲区
        }
    } while (choice < minChoice || choice > maxChoice);
    
    return choice;
}

// ===== 结果显示 =====
void displaySATResult(const SATSolver* solver, SolverResult result) {
    displayPrintSeparator('=', 40);
    printf("          求解完成\n");
    displayPrintSeparator('=', 40);
    
    switch (result) {
        case SOLVER_SAT:
            displaySetColor(COLOR_GREEN);
            printf("结果: SATISFIABLE\n");
            displayResetColor();
            solverPrintSolution(solver);
            break;
        case SOLVER_UNSAT:
            displaySetColor(COLOR_RED);
            printf("结果: UNSATISFIABLE\n");
            displayResetColor();
            break;
        case SOLVER_TIMEOUT:
            displaySetColor(COLOR_YELLOW);
            printf("结果: TIMEOUT\n");
            displayResetColor();
            break;
        case SOLVER_ERROR:
            displaySetColor(COLOR_RED);
            printf("结果: ERROR\n");
            displayResetColor();
            break;
        default:
            printf("结果: UNKNOWN\n");
            break;
    }
    
    displayStatistics(solver);
}

void displayPerformanceComparison(const char* filename) {
    displayPrintHeader("性能对比测试");
    
    // 创建两个求解器实例
    SATSolver* solver1 = solverCreate();
    SATSolver* solver2 = solverCreate();
    CNFParser* parser1 = cnfParserCreate();
    CNFParser* parser2 = cnfParserCreate();
    
    if (!solver1 || !solver2 || !parser1 || !parser2) {
        displayError("内存分配失败");
        goto cleanup;
    }
    
    // 加载同一个文件两次
    if (!cnfParserLoadFile(parser1, filename) || !cnfParserLoadFile(parser2, filename)) {
        displayError("加载CNF文件失败");
        goto cleanup;
    }
    
    CNFFormula* formula1 = cnfParserParse(parser1);
    CNFFormula* formula2 = cnfParserParse(parser2);
    
    if (!formula1 || !formula2) {
        displayError("解析CNF文件失败");
        goto cleanup;
    }
    
    solverLoadFormula(solver1, formula1);
    solverLoadFormula(solver2, formula2);
    
    printf("测试文件: %s\n", filename);
    printf("变量数: %d\n", formula1->numVars);
    printf("子句数: %d\n", formula1->numClauses);
    printf("\n");
    
    // 基础DPLL求解
    displayInfo("基础DPLL求解...");
    solverSetHeuristic(solver1, HEURISTIC_FIRST_UNASSIGNED);
    SolverResult result1 = solverSolve(solver1);
    double time1 = solverGetSolvingTime(solver1);
    
    printf("结果: %s\n", (result1 == SOLVER_SAT) ? "SAT" : "UNSAT");
    printf("时间: %.3f 毫秒\n", time1);
    displayStatistics(solver1);
    printf("\n");
    
    // 优化DPLL求解
    displayInfo("优化DPLL求解...");
    solverSetHeuristic(solver2, HEURISTIC_VSIDS);
    SolverResult result2 = solverSolve(solver2);
    double time2 = solverGetSolvingTime(solver2);
    
    printf("结果: %s\n", (result2 == SOLVER_SAT) ? "SAT" : "UNSAT");
    printf("时间: %.3f 毫秒\n", time2);
    displayStatistics(solver2);
    printf("\n");
    
    // 计算优化率
    if (time1 > 0) {
        double optimizationRate = ((time1 - time2) / time1) * 100.0;
        displayPrintHeader("优化效果");
        printf("基础时间: %.3f 毫秒\n", time1);
        printf("优化时间: %.3f 毫秒\n", time2);
        printf("优化率: %.1f%%\n", optimizationRate);
        
        if (optimizationRate > 0) {
            displaySetColor(COLOR_GREEN);
            printf("优化效果: 提升 %.1f%%\n", optimizationRate);
        } else {
            displaySetColor(COLOR_RED);
            printf("优化效果: 下降 %.1f%%\n", -optimizationRate);
        }
        displayResetColor();
    }
    
cleanup:
    if (solver1) solverDestroy(solver1);
    if (solver2) solverDestroy(solver2);
    if (parser1) cnfParserDestroy(parser1);
    if (parser2) cnfParserDestroy(parser2);
}

void displayStatistics(const SATSolver* solver) {
    if (!solver) return;
    
    displayPrintSeparator('-', 30);
    printf("性能统计\n");
    displayPrintSeparator('-', 30);
    printf("求解时间: %.3f 毫秒\n", solverGetSolvingTime(solver));
    printf("决策次数: %lld\n", solver->stats.decisions);
    printf("冲突次数: %lld\n", solver->stats.conflicts);
    printf("传播次数: %lld\n", solver->stats.propagations);
    printf("回溯次数: %lld\n", solver->stats.backtracks);
}

// ===== 输入输出辅助 =====
bool displayLoadCNFFile(MainController* controller, const char* filename) {
    if (!cnfParserLoadFile(controller->parser, filename)) {
        displayError("加载文件失败: %s", cnfParserGetError(controller->parser));
        return false;
    }
    
    CNFFormula* formula = cnfParserParse(controller->parser);
    if (!formula) {
        displayError("解析文件失败: %s", cnfParserGetError(controller->parser));
        return false;
    }
    
    if (!solverLoadFormula(controller->satSolver, formula)) {
        displayError("加载公式到求解器失败");
        cnfFormulaDestroy(formula);
        return false;
    }
    
    return true;
}

bool displayConfirm(const char* message) {
    char response;
    printf("%s (y/n): ", message);
    if (scanf(" %c", &response) != 1) {
        return false;
    }
    return (response == 'y' || response == 'Y');
}

// ===== 错误处理和消息显示 =====
void displayError(const char* format, ...) {
    displaySetColor(COLOR_RED);
    printf("错误: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    displayResetColor();
}

void displayWarning(const char* format, ...) {
    displaySetColor(COLOR_YELLOW);
    printf("警告: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    displayResetColor();
}

void displayInfo(const char* format, ...) {
    displaySetColor(COLOR_BLUE);
    printf("信息: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    displayResetColor();
}

void displaySuccess(const char* format, ...) {
    displaySetColor(COLOR_GREEN);
    printf("成功: ");
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    displayResetColor();
}

// ===== 格式化输出 =====
void displayPrintSeparator(char c, int length) {
    for (int i = 0; i < length; i++) {
        printf("%c", c);
    }
    printf("\n");
}

void displayPrintHeader(const char* title) {
    int titleLen = strlen(title);
    int totalLen = 60;
    int padding = (totalLen - titleLen) / 2;
    
    displayPrintSeparator('=', totalLen);
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s\n", title);
    displayPrintSeparator('=', totalLen);
}

void displayPrintFormula(const CNFFormula* formula, bool detailed) {
    if (!formula) return;
    
    displayPrintHeader("CNF公式内容");
    printf("p cnf %d %d\n", formula->numVars, formula->numClauses);
    
    if (detailed) {
        for (int i = 0; i < formula->clauses.size; i++) {
            displayPrintClause(&formula->clauses.data[i], i + 1);
        }
    } else {
        printf("... (%d 个子句) ...\n", formula->clauses.size);
    }
}

void displayPrintClause(const Clause* clause, int clauseNum) {
    printf("c%d: ", clauseNum);
    cnfParserPrintClause(clause);
}

// ===== 颜色输出支持 =====
void displaySetColor(int color) {
    if (displaySupportsColor()) {
        printf("\033[%dm", color);
    }
}

void displayResetColor() {
    if (displaySupportsColor()) {
        printf("\033[0m");
    }
}

bool displaySupportsColor() {
    // 简单检测是否支持颜色输出
    char* term = getenv("TERM");
    return term && strstr(term, "color");
}