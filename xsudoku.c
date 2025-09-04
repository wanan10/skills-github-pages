#include "xsudoku.h"

// ===== X数独求解器创建和销毁 =====
SudokuSolver* xsudokuCreate() {
    SudokuSolver* solver = (SudokuSolver*)malloc(sizeof(SudokuSolver));
    if (!solver) return NULL;
    
    solver->sudoku = sudokuGridCreate();
    solver->satSolver = solverCreate();
    solver->difficulty = DIFFICULTY_MEDIUM;
    
    if (!solver->sudoku || !solver->satSolver) {
        xsudokuDestroy(solver);
        return NULL;
    }
    
    return solver;
}

void xsudokuDestroy(SudokuSolver* solver) {
    if (solver) {
        if (solver->sudoku) {
            sudokuGridDestroy(solver->sudoku);
        }
        if (solver->satSolver) {
            solverDestroy(solver->satSolver);
        }
        free(solver);
    }
}

// ===== 数独网格操作 =====
SudokuGrid* sudokuGridCreate() {
    SudokuGrid* grid = (SudokuGrid*)malloc(sizeof(SudokuGrid));
    if (!grid) return NULL;
    
    sudokuGridClear(grid);
    grid->isXSudoku = false;
    
    return grid;
}

void sudokuGridDestroy(SudokuGrid* grid) {
    if (grid) {
        free(grid);
    }
}

void sudokuGridClear(SudokuGrid* grid) {
    if (!grid) return;
    
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            grid->grid[i][j] = 0;
            grid->originalGrid[i][j] = 0;
        }
    }
}

void sudokuGridCopy(SudokuGrid* dest, const SudokuGrid* src) {
    if (!dest || !src) return;
    
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            dest->grid[i][j] = src->grid[i][j];
            dest->originalGrid[i][j] = src->originalGrid[i][j];
        }
    }
    dest->isXSudoku = src->isXSudoku;
}

// ===== 数独加载和显示 =====
bool xsudokuLoadFromFile(SudokuSolver* solver, const char* filename) {
    if (!solver || !filename) return false;
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "错误：无法打开数独文件 %s\n", filename);
        return false;
    }
    
    // 读取是否为X数独标志
    int isXSudoku = 0;
    if (fscanf(file, "%d", &isXSudoku) == 1) {
        solver->sudoku->isXSudoku = (isXSudoku != 0);
    }
    
    // 读取9x9网格
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (fscanf(file, "%d", &solver->sudoku->grid[i][j]) != 1) {
                fprintf(stderr, "错误：数独文件格式不正确\n");
                fclose(file);
                return false;
            }
            solver->sudoku->originalGrid[i][j] = solver->sudoku->grid[i][j];
        }
    }
    
    fclose(file);
    return sudokuGridIsValid(solver->sudoku);
}

void xsudokuLoadFromInput(SudokuSolver* solver) {
    if (!solver) return;
    
    printf("是否为X数独（对角线约束）？(1-是, 0-否): ");
    int isXSudoku;
    if (scanf("%d", &isXSudoku) == 1) {
        solver->sudoku->isXSudoku = (isXSudoku != 0);
    }
    
    printf("请输入9x9数独题目（用0表示空格）：\n");
    for (int i = 0; i < 9; i++) {
        printf("第%d行: ", i + 1);
        for (int j = 0; j < 9; j++) {
            if (scanf("%d", &solver->sudoku->grid[i][j]) != 1) {
                fprintf(stderr, "输入错误\n");
                return;
            }
            solver->sudoku->originalGrid[i][j] = solver->sudoku->grid[i][j];
        }
    }
}

void xsudokuSetGrid(SudokuSolver* solver, int grid[9][9], bool isXSudoku) {
    if (!solver) return;
    
    solver->sudoku->isXSudoku = isXSudoku;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            solver->sudoku->grid[i][j] = grid[i][j];
            solver->sudoku->originalGrid[i][j] = grid[i][j];
        }
    }
}

void xsudokuPrintGrid(const SudokuSolver* solver) {
    if (!solver || !solver->sudoku) return;
    
    const SudokuGrid* grid = solver->sudoku;
    
    printf("┌─────────┬─────────┬─────────┐");
    if (grid->isXSudoku) printf(" (X数独 - 含对角线约束)");
    printf("\n");
    
    for (int i = 0; i < 9; i++) {
        if (i == 3 || i == 6) {
            printf("├─────────┼─────────┼─────────┤\n");
        }
        
        printf("│ ");
        for (int j = 0; j < 9; j++) {
            if (j == 3 || j == 6) {
                printf("│ ");
            }
            
            if (grid->grid[i][j] == 0) {
                // 在X数独中高亮对角线位置
                if (grid->isXSudoku && (xsudokuIsOnMainDiagonal(i, j) || xsudokuIsOnAntiDiagonal(i, j))) {
                    printf("× ");
                } else {
                    printf(". ");
                }
            } else {
                printf("%d ", grid->grid[i][j]);
            }
        }
        printf("│\n");
    }
    printf("└─────────┴─────────┴─────────┘\n");
}

// ===== SAT转换 =====
int xsudokuGetVarIndex(int row, int col, int num) {
    return row * 81 + col * 9 + num;
}

bool xsudokuGenerateSATClauses(SudokuSolver* solver) {
    if (!solver || !solver->satSolver) return false;
    
    // 创建CNF公式
    CNFFormula* formula = cnfFormulaCreate();
    if (!formula) return false;
    
    formula->numVars = 729; // 9*9*9个变量
    
    // 添加各种约束
    xsudokuAddCellConstraints(formula);
    xsudokuAddRowConstraints(formula);
    xsudokuAddColumnConstraints(formula);
    xsudokuAddBlockConstraints(formula);
    
    // 如果是X数独，添加对角线约束
    if (solver->sudoku->isXSudoku) {
        xsudokuAddXConstraints(formula);
    }
    
    // 添加已知值约束
    xsudokuAddKnownValues(formula, solver->sudoku);
    
    formula->numClauses = formula->clauses.size;
    
    // 加载到SAT求解器
    if (!solverLoadFormula(solver->satSolver, formula)) {
        cnfFormulaDestroy(formula);
        return false;
    }
    
    return true;
}

void xsudokuAddCellConstraints(CNFFormula* formula) {
    // 每个格子必须有且仅有一个数字
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            // 至少有一个数字
            Clause atLeastOne = createClause(formula->clauses.size + 1);
            for (int n = 1; n <= 9; n++) {
                clauseAddLiteral(&atLeastOne, createLiteral(xsudokuGetVarIndex(r, c, n), true));
            }
            cnfFormulaAddClause(formula, atLeastOne);
            
            // 至多有一个数字
            for (int n1 = 1; n1 <= 9; n1++) {
                for (int n2 = n1 + 1; n2 <= 9; n2++) {
                    Clause atMostOne = createClause(formula->clauses.size + 1);
                    clauseAddLiteral(&atMostOne, createLiteral(xsudokuGetVarIndex(r, c, n1), false));
                    clauseAddLiteral(&atMostOne, createLiteral(xsudokuGetVarIndex(r, c, n2), false));
                    cnfFormulaAddClause(formula, atMostOne);
                }
            }
        }
    }
}

void xsudokuAddRowConstraints(CNFFormula* formula) {
    // 每行每个数字恰好出现一次
    for (int r = 0; r < 9; r++) {
        for (int n = 1; n <= 9; n++) {
            // 至少出现一次
            Clause atLeastOne = createClause(formula->clauses.size + 1);
            for (int c = 0; c < 9; c++) {
                clauseAddLiteral(&atLeastOne, createLiteral(xsudokuGetVarIndex(r, c, n), true));
            }
            cnfFormulaAddClause(formula, atLeastOne);
            
            // 至多出现一次
            for (int c1 = 0; c1 < 9; c1++) {
                for (int c2 = c1 + 1; c2 < 9; c2++) {
                    Clause atMostOne = createClause(formula->clauses.size + 1);
                    clauseAddLiteral(&atMostOne, createLiteral(xsudokuGetVarIndex(r, c1, n), false));
                    clauseAddLiteral(&atMostOne, createLiteral(xsudokuGetVarIndex(r, c2, n), false));
                    cnfFormulaAddClause(formula, atMostOne);
                }
            }
        }
    }
}

void xsudokuAddColumnConstraints(CNFFormula* formula) {
    // 每列每个数字恰好出现一次
    for (int c = 0; c < 9; c++) {
        for (int n = 1; n <= 9; n++) {
            // 至少出现一次
            Clause atLeastOne = createClause(formula->clauses.size + 1);
            for (int r = 0; r < 9; r++) {
                clauseAddLiteral(&atLeastOne, createLiteral(xsudokuGetVarIndex(r, c, n), true));
            }
            cnfFormulaAddClause(formula, atLeastOne);
            
            // 至多出现一次
            for (int r1 = 0; r1 < 9; r1++) {
                for (int r2 = r1 + 1; r2 < 9; r2++) {
                    Clause atMostOne = createClause(formula->clauses.size + 1);
                    clauseAddLiteral(&atMostOne, createLiteral(xsudokuGetVarIndex(r1, c, n), false));
                    clauseAddLiteral(&atMostOne, createLiteral(xsudokuGetVarIndex(r2, c, n), false));
                    cnfFormulaAddClause(formula, atMostOne);
                }
            }
        }
    }
}

void xsudokuAddBlockConstraints(CNFFormula* formula) {
    // 每个3x3子网格每个数字恰好出现一次
    for (int boxR = 0; boxR < 3; boxR++) {
        for (int boxC = 0; boxC < 3; boxC++) {
            for (int n = 1; n <= 9; n++) {
                // 至少出现一次
                Clause atLeastOne = createClause(formula->clauses.size + 1);
                for (int r = boxR * 3; r < (boxR + 1) * 3; r++) {
                    for (int c = boxC * 3; c < (boxC + 1) * 3; c++) {
                        clauseAddLiteral(&atLeastOne, createLiteral(xsudokuGetVarIndex(r, c, n), true));
                    }
                }
                cnfFormulaAddClause(formula, atLeastOne);
                
                // 至多出现一次
                int positions[9];
                int posCount = 0;
                for (int r = boxR * 3; r < (boxR + 1) * 3; r++) {
                    for (int c = boxC * 3; c < (boxC + 1) * 3; c++) {
                        positions[posCount++] = xsudokuGetVarIndex(r, c, n);
                    }
                }
                
                for (int i = 0; i < posCount; i++) {
                    for (int j = i + 1; j < posCount; j++) {
                        Clause atMostOne = createClause(formula->clauses.size + 1);
                        clauseAddLiteral(&atMostOne, createLiteral(positions[i], false));
                        clauseAddLiteral(&atMostOne, createLiteral(positions[j], false));
                        cnfFormulaAddClause(formula, atMostOne);
                    }
                }
            }
        }
    }
}

void xsudokuAddXConstraints(CNFFormula* formula) {
    // 主对角线约束
    for (int n = 1; n <= 9; n++) {
        // 至少出现一次
        Clause atLeastOne = createClause(formula->clauses.size + 1);
        for (int i = 0; i < 9; i++) {
            clauseAddLiteral(&atLeastOne, createLiteral(xsudokuGetVarIndex(i, i, n), true));
        }
        cnfFormulaAddClause(formula, atLeastOne);
        
        // 至多出现一次
        for (int i1 = 0; i1 < 9; i1++) {
            for (int i2 = i1 + 1; i2 < 9; i2++) {
                Clause atMostOne = createClause(formula->clauses.size + 1);
                clauseAddLiteral(&atMostOne, createLiteral(xsudokuGetVarIndex(i1, i1, n), false));
                clauseAddLiteral(&atMostOne, createLiteral(xsudokuGetVarIndex(i2, i2, n), false));
                cnfFormulaAddClause(formula, atMostOne);
            }
        }
    }
    
    // 反对角线约束
    for (int n = 1; n <= 9; n++) {
        // 至少出现一次
        Clause atLeastOne = createClause(formula->clauses.size + 1);
        for (int i = 0; i < 9; i++) {
            clauseAddLiteral(&atLeastOne, createLiteral(xsudokuGetVarIndex(i, 8-i, n), true));
        }
        cnfFormulaAddClause(formula, atLeastOne);
        
        // 至多出现一次
        for (int i1 = 0; i1 < 9; i1++) {
            for (int i2 = i1 + 1; i2 < 9; i2++) {
                Clause atMostOne = createClause(formula->clauses.size + 1);
                clauseAddLiteral(&atMostOne, createLiteral(xsudokuGetVarIndex(i1, 8-i1, n), false));
                clauseAddLiteral(&atMostOne, createLiteral(xsudokuGetVarIndex(i2, 8-i2, n), false));
                cnfFormulaAddClause(formula, atMostOne);
            }
        }
    }
}

void xsudokuAddKnownValues(CNFFormula* formula, const SudokuGrid* grid) {
    // 已知数字的约束
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            if (grid->grid[r][c] != 0) {
                Clause knownValue = createClause(formula->clauses.size + 1);
                clauseAddLiteral(&knownValue, createLiteral(xsudokuGetVarIndex(r, c, grid->grid[r][c]), true));
                cnfFormulaAddClause(formula, knownValue);
            }
        }
    }
}

// ===== 数独求解 =====
bool xsudokuSolve(SudokuSolver* solver) {
    if (!solver) return false;
    
    printf("正在生成SAT约束...\n");
    if (!xsudokuGenerateSATClauses(solver)) {
        printf("生成SAT约束失败！\n");
        return false;
    }
    
    printf("生成了 %d 个子句", solver->satSolver->formula->numClauses);
    if (solver->sudoku->isXSudoku) {
        printf("（包含X约束）");
    }
    printf("\n开始求解...\n");
    
    // 使用启发式求解
    solverSetHeuristic(solver->satSolver, HEURISTIC_VSIDS);
    SolverResult result = solverSolve(solver->satSolver);
    
    if (result == SOLVER_SAT) {
        printf("数独求解成功！\n");
        xsudokuExtractSolution(solver);
        return true;
    } else {
        printf("数独无解！\n");
        return false;
    }
}

void xsudokuExtractSolution(SudokuSolver* solver) {
    if (!solver || !solver->satSolver || !solver->satSolver->assignment) return;
    
    // 从SAT解中提取数独解
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            for (int n = 1; n <= 9; n++) {
                int varIndex = xsudokuGetVarIndex(r, c, n);
                if (solver->satSolver->assignment[varIndex] == SAT_TRUE) {
                    solver->sudoku->grid[r][c] = n;
                    break;
                }
            }
        }
    }
}

// ===== 工具函数 =====
bool sudokuGridIsValid(const SudokuGrid* grid) {
    if (!grid) return false;
    
    // 检查所有值都在0-9范围内
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (grid->grid[i][j] < 0 || grid->grid[i][j] > 9) {
                return false;
            }
        }
    }
    
    return xsudokuCheckConstraints(grid, grid->isXSudoku);
}

bool xsudokuCheckConstraints(const SudokuGrid* grid, bool isXSudoku) {
    if (!grid) return false;
    
    // 检查行约束
    for (int r = 0; r < 9; r++) {
        bool used[10] = {false};
        for (int c = 0; c < 9; c++) {
            int val = grid->grid[r][c];
            if (val != 0) {
                if (used[val]) return false;
                used[val] = true;
            }
        }
    }
    
    // 检查列约束
    for (int c = 0; c < 9; c++) {
        bool used[10] = {false};
        for (int r = 0; r < 9; r++) {
            int val = grid->grid[r][c];
            if (val != 0) {
                if (used[val]) return false;
                used[val] = true;
            }
        }
    }
    
    // 检查3x3块约束
    for (int boxR = 0; boxR < 3; boxR++) {
        for (int boxC = 0; boxC < 3; boxC++) {
            bool used[10] = {false};
            for (int r = boxR * 3; r < (boxR + 1) * 3; r++) {
                for (int c = boxC * 3; c < (boxC + 1) * 3; c++) {
                    int val = grid->grid[r][c];
                    if (val != 0) {
                        if (used[val]) return false;
                        used[val] = true;
                    }
                }
            }
        }
    }
    
    // 如果是X数独，检查对角线约束
    if (isXSudoku) {
        // 主对角线
        bool used[10] = {false};
        for (int i = 0; i < 9; i++) {
            int val = grid->grid[i][i];
            if (val != 0) {
                if (used[val]) return false;
                used[val] = true;
            }
        }
        
        // 反对角线
        memset(used, false, sizeof(used));
        for (int i = 0; i < 9; i++) {
            int val = grid->grid[i][8-i];
            if (val != 0) {
                if (used[val]) return false;
                used[val] = true;
            }
        }
    }
    
    return true;
}

bool xsudokuIsOnMainDiagonal(int row, int col) {
    return row == col;
}

bool xsudokuIsOnAntiDiagonal(int row, int col) {
    return row + col == 8;
}

// ===== 交互式游戏 =====
void xsudokuPlayGame(SudokuSolver* solver) {
    if (!solver) return;
    
    printf("=== X数独游戏 ===\n");
    printf("选择输入方式：\n");
    printf("1. 从文件读取\n");
    printf("2. 手动输入\n");
    printf("3. 使用示例题目\n");
    printf("4. 生成新题目\n");
    
    int choice = xsudokuGetUserChoice();
    
    switch (choice) {
        case 1: {
            printf("请输入文件名: ");
            char filename[256];
            if (scanf("%s", filename) == 1) {
                xsudokuLoadFromFile(solver, filename);
            }
            break;
        }
        case 2:
            xsudokuLoadFromInput(solver);
            break;
        case 3: {
            // 示例X数独题目
            int example[9][9] = {
                {5, 3, 0, 0, 7, 0, 0, 0, 0},
                {6, 0, 0, 1, 9, 5, 0, 0, 0},
                {0, 9, 8, 0, 0, 0, 0, 6, 0},
                {8, 0, 0, 0, 6, 0, 0, 0, 3},
                {4, 0, 0, 8, 0, 3, 0, 0, 1},
                {7, 0, 0, 0, 2, 0, 0, 0, 6},
                {0, 6, 0, 0, 0, 0, 2, 8, 0},
                {0, 0, 0, 4, 1, 9, 0, 0, 5},
                {0, 0, 0, 0, 8, 0, 0, 7, 9}
            };
            
            printf("是否使用X约束？(1-是, 0-否): ");
            int useX = 0;
            if (scanf("%d", &useX) != 1) {
                useX = 0;
            }
            xsudokuSetGrid(solver, example, useX != 0);
            break;
        }
        case 4:
            printf("题目生成功能开发中...\n");
            return;
        default:
            printf("无效选择，使用示例题目\n");
            break;
    }
    
    printf("\n原始题目：\n");
    xsudokuPrintGrid(solver);
    
    clock_t start = clock();
    bool solved = xsudokuSolve(solver);
    clock_t end = clock();
    
    if (solved) {
        printf("\n解答：\n");
        xsudokuPrintGrid(solver);
        
        double time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
        printf("\n求解时间: %.2f 毫秒\n", time);
        
        solverPrintStatistics(solver->satSolver);
    }
}

int xsudokuGetUserChoice() {
    int choice;
    printf("请选择 (1-4): ");
    if (scanf("%d", &choice) != 1) {
        choice = 3; // 默认选择
    }
    return choice;
}