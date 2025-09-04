#include "sat_solver_c.h"

// ===== 数独求解器创建和销毁 =====
SudokuSolver* sudokuSolverCreate() {
    SudokuSolver* solver = (SudokuSolver*)malloc(sizeof(SudokuSolver));
    if (!solver) return NULL;
    
    // 初始化数独网格
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            solver->grid[i][j] = 0;
        }
    }
    
    solver->satSolver = satSolverCreate();
    if (!solver->satSolver) {
        free(solver);
        return NULL;
    }
    
    return solver;
}

void sudokuSolverDestroy(SudokuSolver* solver) {
    if (solver) {
        if (solver->satSolver) {
            satSolverDestroy(solver->satSolver);
        }
        free(solver);
    }
}

// ===== 变量索引映射 =====
int sudokuSolverGetVarIndex(int row, int col, int num) {
    // 将(row, col, num)映射到SAT变量编号
    // 变量编号从1开始，范围：1 to 729 (9*9*9)
    return row * 81 + col * 9 + num;
}

// ===== 数独输入输出 =====
void sudokuSolverLoadFromFile(SudokuSolver* solver, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "错误：无法打开数独文件 %s\n", filename);
        return;
    }
    
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (fscanf(file, "%d", &solver->grid[i][j]) != 1) {
                fprintf(stderr, "错误：数独文件格式不正确\n");
                fclose(file);
                return;
            }
        }
    }
    
    fclose(file);
}

void sudokuSolverLoadFromInput(SudokuSolver* solver) {
    printf("请输入9x9数独题目（用0表示空格）：\n");
    for (int i = 0; i < 9; i++) {
        printf("第%d行: ", i + 1);
        for (int j = 0; j < 9; j++) {
            if (scanf("%d", &solver->grid[i][j]) != 1) {
                fprintf(stderr, "输入错误\n");
                return;
            }
        }
    }
}

void sudokuSolverPrintGrid(const SudokuSolver* solver) {
    printf("┌─────────┬─────────┬─────────┐\n");
    for (int i = 0; i < 9; i++) {
        if (i == 3 || i == 6) {
            printf("├─────────┼─────────┼─────────┤\n");
        }
        
        printf("│ ");
        for (int j = 0; j < 9; j++) {
            if (j == 3 || j == 6) {
                printf("│ ");
            }
            
            if (solver->grid[i][j] == 0) {
                printf(". ");
            } else {
                printf("%d ", solver->grid[i][j]);
            }
        }
        printf("│\n");
    }
    printf("└─────────┴─────────┴─────────┘\n");
}

// ===== SAT约束生成 =====
void sudokuSolverGenerateSATClauses(SudokuSolver* solver) {
    CNFFormula* formula = &solver->satSolver->formula;
    formula->numVars = 729; // 9*9*9个变量
    
    // 初始化SAT求解器的赋值数组
    if (solver->satSolver->assignment) {
        free(solver->satSolver->assignment);
    }
    solver->satSolver->assignment = (int*)calloc(formula->numVars + 1, sizeof(int));
    
    // 约束1：每个格子必须有且仅有一个数字
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            // 至少有一个数字
            Clause atLeastOne = createClause();
            for (int n = 1; n <= 9; n++) {
                clauseAddLiteral(&atLeastOne, createLiteral(sudokuSolverGetVarIndex(r, c, n), true));
            }
            formulaAddClause(formula, atLeastOne);
            
            // 至多有一个数字
            for (int n1 = 1; n1 <= 9; n1++) {
                for (int n2 = n1 + 1; n2 <= 9; n2++) {
                    Clause atMostOne = createClause();
                    clauseAddLiteral(&atMostOne, createLiteral(sudokuSolverGetVarIndex(r, c, n1), false));
                    clauseAddLiteral(&atMostOne, createLiteral(sudokuSolverGetVarIndex(r, c, n2), false));
                    formulaAddClause(formula, atMostOne);
                }
            }
        }
    }
    
    // 约束2：每行每个数字恰好出现一次
    for (int r = 0; r < 9; r++) {
        for (int n = 1; n <= 9; n++) {
            // 至少出现一次
            Clause atLeastOne = createClause();
            for (int c = 0; c < 9; c++) {
                clauseAddLiteral(&atLeastOne, createLiteral(sudokuSolverGetVarIndex(r, c, n), true));
            }
            formulaAddClause(formula, atLeastOne);
            
            // 至多出现一次
            for (int c1 = 0; c1 < 9; c1++) {
                for (int c2 = c1 + 1; c2 < 9; c2++) {
                    Clause atMostOne = createClause();
                    clauseAddLiteral(&atMostOne, createLiteral(sudokuSolverGetVarIndex(r, c1, n), false));
                    clauseAddLiteral(&atMostOne, createLiteral(sudokuSolverGetVarIndex(r, c2, n), false));
                    formulaAddClause(formula, atMostOne);
                }
            }
        }
    }
    
    // 约束3：每列每个数字恰好出现一次
    for (int c = 0; c < 9; c++) {
        for (int n = 1; n <= 9; n++) {
            // 至少出现一次
            Clause atLeastOne = createClause();
            for (int r = 0; r < 9; r++) {
                clauseAddLiteral(&atLeastOne, createLiteral(sudokuSolverGetVarIndex(r, c, n), true));
            }
            formulaAddClause(formula, atLeastOne);
            
            // 至多出现一次
            for (int r1 = 0; r1 < 9; r1++) {
                for (int r2 = r1 + 1; r2 < 9; r2++) {
                    Clause atMostOne = createClause();
                    clauseAddLiteral(&atMostOne, createLiteral(sudokuSolverGetVarIndex(r1, c, n), false));
                    clauseAddLiteral(&atMostOne, createLiteral(sudokuSolverGetVarIndex(r2, c, n), false));
                    formulaAddClause(formula, atMostOne);
                }
            }
        }
    }
    
    // 约束4：每个3x3子网格每个数字恰好出现一次
    for (int boxR = 0; boxR < 3; boxR++) {
        for (int boxC = 0; boxC < 3; boxC++) {
            for (int n = 1; n <= 9; n++) {
                // 至少出现一次
                Clause atLeastOne = createClause();
                for (int r = boxR * 3; r < (boxR + 1) * 3; r++) {
                    for (int c = boxC * 3; c < (boxC + 1) * 3; c++) {
                        clauseAddLiteral(&atLeastOne, createLiteral(sudokuSolverGetVarIndex(r, c, n), true));
                    }
                }
                formulaAddClause(formula, atLeastOne);
                
                // 至多出现一次
                int positions[9];
                int posCount = 0;
                for (int r = boxR * 3; r < (boxR + 1) * 3; r++) {
                    for (int c = boxC * 3; c < (boxC + 1) * 3; c++) {
                        positions[posCount++] = sudokuSolverGetVarIndex(r, c, n);
                    }
                }
                
                for (int i = 0; i < posCount; i++) {
                    for (int j = i + 1; j < posCount; j++) {
                        Clause atMostOne = createClause();
                        clauseAddLiteral(&atMostOne, createLiteral(positions[i], false));
                        clauseAddLiteral(&atMostOne, createLiteral(positions[j], false));
                        formulaAddClause(formula, atMostOne);
                    }
                }
            }
        }
    }
    
    // 约束5：已知数字的约束
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            if (solver->grid[r][c] != 0) {
                Clause knownValue = createClause();
                clauseAddLiteral(&knownValue, createLiteral(sudokuSolverGetVarIndex(r, c, solver->grid[r][c]), true));
                formulaAddClause(formula, knownValue);
            }
        }
    }
    
    formula->numClauses = formula->clauses.size;
}

// ===== 解提取 =====
void sudokuSolverExtractSolution(SudokuSolver* solver) {
    // 从SAT解中提取数独解
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            for (int n = 1; n <= 9; n++) {
                int varIndex = sudokuSolverGetVarIndex(r, c, n);
                if (solver->satSolver->assignment[varIndex] == TRUE_VAL) {
                    solver->grid[r][c] = n;
                    break;
                }
            }
        }
    }
}

// ===== 数独求解 =====
bool sudokuSolverSolve(SudokuSolver* solver) {
    printf("正在生成SAT约束...\n");
    sudokuSolverGenerateSATClauses(solver);
    
    printf("生成了 %d 个子句\n", solver->satSolver->formula.numClauses);
    printf("开始求解...\n");
    
    bool result = satSolverSolve(solver->satSolver, true); // 使用优化
    
    if (result) {
        printf("数独求解成功！\n");
        sudokuSolverExtractSolution(solver);
        return true;
    } else {
        printf("数独无解！\n");
        return false;
    }
}

// ===== 交互式游戏 =====
void sudokuSolverPlayInteractiveGame(SudokuSolver* solver) {
    printf("=== 数独游戏 ===\n");
    printf("选择输入方式：\n");
    printf("1. 从文件读取\n");
    printf("2. 手动输入\n");
    printf("3. 使用示例题目\n");
    
    int choice;
    printf("请选择 (1-3): ");
    if (scanf("%d", &choice) != 1) {
        fprintf(stderr, "输入错误\n");
        return;
    }
    
    switch (choice) {
        case 1: {
            printf("请输入文件名: ");
            char filename[256];
            if (scanf("%s", filename) == 1) {
                sudokuSolverLoadFromFile(solver, filename);
            }
            break;
        }
        case 2:
            sudokuSolverLoadFromInput(solver);
            break;
        case 3: {
            // 示例数独题目
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
            
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    solver->grid[i][j] = example[i][j];
                }
            }
            break;
        }
        default:
            printf("无效选择，使用示例题目\n");
            break;
    }
    
    printf("\n原始题目：\n");
    sudokuSolverPrintGrid(solver);
    
    clock_t start = clock();
    bool solved = sudokuSolverSolve(solver);
    clock_t end = clock();
    
    if (solved) {
        printf("\n解答：\n");
        sudokuSolverPrintGrid(solver);
        
        double time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
        printf("\n求解时间: %.2f 毫秒\n", time);
        
        satSolverPrintStatistics(solver->satSolver);
    }
}