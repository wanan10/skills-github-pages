#ifndef DISPLAY_H
#define DISPLAY_H

#include "common.h"
#include "cnfparser.h"
#include "solver.h"
#include "xsudoku.h"

// 程序模式枚举
typedef enum {
    MODE_SAT_SOLVE = 1,
    MODE_SUDOKU_GAME = 2,
    MODE_PERFORMANCE_TEST = 3,
    MODE_BATCH_SOLVE = 4,
    MODE_INTERACTIVE = 5
} ProgramMode;

// 程序配置结构
typedef struct {
    char* inputFile;
    char* outputFile;
    bool useOptimization;
    bool verifyResult;
    bool showFormula;
    bool showStatistics;
    bool quietMode;
    HeuristicType heuristic;
    ProgramMode mode;
} ProgramConfig;

// 主控制器结构
typedef struct {
    ProgramConfig config;
    CNFParser* parser;
    SATSolver* satSolver;
    SudokuSolver* sudokuSolver;
} MainController;

// 主控制器函数
MainController* displayCreate();
void displayDestroy(MainController* controller);
int displayRun(MainController* controller, int argc, char* argv[]);

// 命令行解析
bool displayParseCommandLine(MainController* controller, int argc, char* argv[]);
void displayPrintUsage(const char* programName);
void displayPrintVersion();

// 主要功能模块
int displaySATSolveMode(MainController* controller);
int displaySudokuGameMode(MainController* controller);
int displayPerformanceTestMode(MainController* controller);
int displayBatchSolveMode(MainController* controller);
int displayInteractiveMode(MainController* controller);

// 交互界面
void displayShowMainMenu();
int displayGetUserChoice(const char* prompt, int minChoice, int maxChoice);
void displayShowSATMenu();
void displayShowSudokuMenu();
void displayShowPerformanceMenu();

// 结果显示
void displaySATResult(const SATSolver* solver, SolverResult result);
void displaySudokuResult(const SudokuSolver* solver, bool solved);
void displayPerformanceComparison(const char* filename);
void displayStatistics(const SATSolver* solver);

// 输入输出辅助
bool displayLoadCNFFile(MainController* controller, const char* filename);
void displaySaveResult(const SATSolver* solver, const char* filename);
bool displayConfirm(const char* message);
void displayProgress(const char* message, int current, int total);

// 错误处理和消息显示
void displayError(const char* format, ...);
void displayWarning(const char* format, ...);
void displayInfo(const char* format, ...);
void displaySuccess(const char* format, ...);

// 格式化输出
void displayPrintSeparator(char c, int length);
void displayPrintHeader(const char* title);
void displayPrintFormula(const CNFFormula* formula, bool detailed);
void displayPrintClause(const Clause* clause, int clauseNum);

// 颜色输出支持（如果终端支持）
void displaySetColor(int color);
void displayResetColor();
bool displaySupportsColor();

// 颜色常量
#define COLOR_RED     31
#define COLOR_GREEN   32
#define COLOR_YELLOW  33
#define COLOR_BLUE    34
#define COLOR_MAGENTA 35
#define COLOR_CYAN    36
#define COLOR_WHITE   37

#endif // DISPLAY_H