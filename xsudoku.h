#ifndef XSUDOKU_H
#define XSUDOKU_H

#include "common.h"
#include "solver.h"

// 数独难度级别
typedef enum {
    DIFFICULTY_EASY = 1,
    DIFFICULTY_MEDIUM = 2,
    DIFFICULTY_HARD = 3,
    DIFFICULTY_EXPERT = 4,
    DIFFICULTY_EXTREME = 5
} SudokuDifficulty;

// 数独生成选项
typedef struct {
    bool includeXConstraint;    // 是否包含对角线约束（X数独）
    SudokuDifficulty difficulty;
    int minClues;              // 最少提示数
    int maxClues;              // 最多提示数
    bool symmetric;            // 是否对称生成
} SudokuGenerateOptions;

// X数独求解器函数声明
SudokuSolver* xsudokuCreate();
void xsudokuDestroy(SudokuSolver* solver);

// 数独网格操作
SudokuGrid* sudokuGridCreate();
void sudokuGridDestroy(SudokuGrid* grid);
void sudokuGridClear(SudokuGrid* grid);
void sudokuGridCopy(SudokuGrid* dest, const SudokuGrid* src);
bool sudokuGridIsValid(const SudokuGrid* grid);

// 数独加载和保存
bool xsudokuLoadFromFile(SudokuSolver* solver, const char* filename);
bool xsudokuSaveToFile(const SudokuSolver* solver, const char* filename);
void xsudokuLoadFromInput(SudokuSolver* solver);
void xsudokuSetGrid(SudokuSolver* solver, int grid[9][9], bool isXSudoku);

// 数独显示
void xsudokuPrintGrid(const SudokuSolver* solver);
void xsudokuPrintGridWithHighlight(const SudokuSolver* solver, int row, int col);
void xsudokuPrintSolution(const SudokuSolver* solver);

// 数独生成
bool xsudokuGenerate(SudokuSolver* solver, const SudokuGenerateOptions* options);
bool xsudokuGenerateComplete(SudokuGrid* grid, bool isXSudoku);
void xsudokuRemoveClues(SudokuGrid* grid, const SudokuGenerateOptions* options);
int xsudokuCountSolutions(const SudokuGrid* grid, bool isXSudoku);

// SAT转换
bool xsudokuGenerateSATClauses(SudokuSolver* solver);
int xsudokuGetVarIndex(int row, int col, int num);
void xsudokuAddCellConstraints(CNFFormula* formula);
void xsudokuAddRowConstraints(CNFFormula* formula);
void xsudokuAddColumnConstraints(CNFFormula* formula);
void xsudokuAddBlockConstraints(CNFFormula* formula);
void xsudokuAddXConstraints(CNFFormula* formula);  // 对角线约束
void xsudokuAddKnownValues(CNFFormula* formula, const SudokuGrid* grid);

// 数独求解
bool xsudokuSolve(SudokuSolver* solver);
void xsudokuExtractSolution(SudokuSolver* solver);
bool xsudokuHasUniqueSolution(const SudokuGrid* grid, bool isXSudoku);

// 数独验证
bool xsudokuIsValidMove(const SudokuGrid* grid, int row, int col, int num);
bool xsudokuIsComplete(const SudokuGrid* grid);
bool xsudokuCheckConstraints(const SudokuGrid* grid, bool isXSudoku);

// 交互式游戏
void xsudokuPlayGame(SudokuSolver* solver);
void xsudokuShowMenu();
int xsudokuGetUserChoice();
void xsudokuHandleUserMove(SudokuSolver* solver);
void xsudokuShowHint(const SudokuSolver* solver);

// 工具函数
bool xsudokuIsInSameRow(int row1, int col1, int row2, int col2);
bool xsudokuIsInSameColumn(int row1, int col1, int row2, int col2);
bool xsudokuIsInSameBlock(int row1, int col1, int row2, int col2);
bool xsudokuIsOnMainDiagonal(int row, int col);
bool xsudokuIsOnAntiDiagonal(int row, int col);

// 难度评估
SudokuDifficulty xsudokuEvaluateDifficulty(const SudokuGrid* grid, bool isXSudoku);
int xsudokuCountClues(const SudokuGrid* grid);
int xsudokuCountEmptyCells(const SudokuGrid* grid);

#endif // XSUDOKU_H