#include "sat_solver.h"

// SudokuSolver实现
SudokuSolver::SudokuSolver() {
    // 初始化数独网格
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            grid[i][j] = 0;
        }
    }
}

int SudokuSolver::getVarIndex(int row, int col, int num) const {
    // 将(row, col, num)映射到SAT变量编号
    // 变量编号从1开始，范围：1 to 729 (9*9*9)
    return row * 81 + col * 9 + num;
}

void SudokuSolver::generateSATClauses() {
    CNFFormula& formula = satSolver.formula;
    formula.numVars = 729; // 9*9*9个变量
    formula.numClauses = 0;
    
    // 约束1：每个格子必须有且仅有一个数字
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            // 至少有一个数字
            Clause atLeastOne;
            for (int n = 1; n <= 9; n++) {
                atLeastOne.addLiteral(Literal(getVarIndex(r, c, n), true));
            }
            formula.addClause(atLeastOne);
            
            // 至多有一个数字
            for (int n1 = 1; n1 <= 9; n1++) {
                for (int n2 = n1 + 1; n2 <= 9; n2++) {
                    Clause atMostOne;
                    atMostOne.addLiteral(Literal(getVarIndex(r, c, n1), false));
                    atMostOne.addLiteral(Literal(getVarIndex(r, c, n2), false));
                    formula.addClause(atMostOne);
                }
            }
        }
    }
    
    // 约束2：每行每个数字恰好出现一次
    for (int r = 0; r < 9; r++) {
        for (int n = 1; n <= 9; n++) {
            // 至少出现一次
            Clause atLeastOne;
            for (int c = 0; c < 9; c++) {
                atLeastOne.addLiteral(Literal(getVarIndex(r, c, n), true));
            }
            formula.addClause(atLeastOne);
            
            // 至多出现一次
            for (int c1 = 0; c1 < 9; c1++) {
                for (int c2 = c1 + 1; c2 < 9; c2++) {
                    Clause atMostOne;
                    atMostOne.addLiteral(Literal(getVarIndex(r, c1, n), false));
                    atMostOne.addLiteral(Literal(getVarIndex(r, c2, n), false));
                    formula.addClause(atMostOne);
                }
            }
        }
    }
    
    // 约束3：每列每个数字恰好出现一次
    for (int c = 0; c < 9; c++) {
        for (int n = 1; n <= 9; n++) {
            // 至少出现一次
            Clause atLeastOne;
            for (int r = 0; r < 9; r++) {
                atLeastOne.addLiteral(Literal(getVarIndex(r, c, n), true));
            }
            formula.addClause(atLeastOne);
            
            // 至多出现一次
            for (int r1 = 0; r1 < 9; r1++) {
                for (int r2 = r1 + 1; r2 < 9; r2++) {
                    Clause atMostOne;
                    atMostOne.addLiteral(Literal(getVarIndex(r1, c, n), false));
                    atMostOne.addLiteral(Literal(getVarIndex(r2, c, n), false));
                    formula.addClause(atMostOne);
                }
            }
        }
    }
    
    // 约束4：每个3x3子网格每个数字恰好出现一次
    for (int boxR = 0; boxR < 3; boxR++) {
        for (int boxC = 0; boxC < 3; boxC++) {
            for (int n = 1; n <= 9; n++) {
                // 至少出现一次
                Clause atLeastOne;
                for (int r = boxR * 3; r < (boxR + 1) * 3; r++) {
                    for (int c = boxC * 3; c < (boxC + 1) * 3; c++) {
                        atLeastOne.addLiteral(Literal(getVarIndex(r, c, n), true));
                    }
                }
                formula.addClause(atLeastOne);
                
                // 至多出现一次
                DynamicArray<int> positions;
                for (int r = boxR * 3; r < (boxR + 1) * 3; r++) {
                    for (int c = boxC * 3; c < (boxC + 1) * 3; c++) {
                        positions.push_back(getVarIndex(r, c, n));
                    }
                }
                
                for (int i = 0; i < positions.getSize(); i++) {
                    for (int j = i + 1; j < positions.getSize(); j++) {
                        Clause atMostOne;
                        atMostOne.addLiteral(Literal(positions[i], false));
                        atMostOne.addLiteral(Literal(positions[j], false));
                        formula.addClause(atMostOne);
                    }
                }
            }
        }
    }
    
    // 约束5：已知数字的约束
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            if (grid[r][c] != 0) {
                Clause knownValue;
                knownValue.addLiteral(Literal(getVarIndex(r, c, grid[r][c]), true));
                formula.addClause(knownValue);
            }
        }
    }
    
    formula.numClauses = formula.clauses.getSize();
    
    // 初始化SAT求解器的赋值数组
    if (satSolver.assignment) {
        delete[] satSolver.assignment;
    }
    satSolver.assignment = new int[formula.numVars + 1];
    for (int i = 0; i <= formula.numVars; i++) {
        satSolver.assignment[i] = UNASSIGNED;
    }
}

void SudokuSolver::extractSolution() {
    // 从SAT解中提取数独解
    for (int r = 0; r < 9; r++) {
        for (int c = 0; c < 9; c++) {
            for (int n = 1; n <= 9; n++) {
                int varIndex = getVarIndex(r, c, n);
                if (satSolver.assignment[varIndex] == TRUE) {
                    grid[r][c] = n;
                    break;
                }
            }
        }
    }
}

void SudokuSolver::loadSudoku(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "错误：无法打开数独文件 " << filename << std::endl;
        return;
    }
    
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            file >> grid[i][j];
        }
    }
    
    file.close();
}

void SudokuSolver::loadSudokuFromInput() {
    std::cout << "请输入9x9数独题目（用0表示空格）：" << std::endl;
    for (int i = 0; i < 9; i++) {
        std::cout << "第" << (i + 1) << "行: ";
        for (int j = 0; j < 9; j++) {
            std::cin >> grid[i][j];
        }
    }
}

void SudokuSolver::printGrid() const {
    std::cout << "┌─────────┬─────────┬─────────┐" << std::endl;
    for (int i = 0; i < 9; i++) {
        if (i == 3 || i == 6) {
            std::cout << "├─────────┼─────────┼─────────┤" << std::endl;
        }
        
        std::cout << "│ ";
        for (int j = 0; j < 9; j++) {
            if (j == 3 || j == 6) {
                std::cout << "│ ";
            }
            
            if (grid[i][j] == 0) {
                std::cout << ". ";
            } else {
                std::cout << grid[i][j] << " ";
            }
        }
        std::cout << "│" << std::endl;
    }
    std::cout << "└─────────┴─────────┴─────────┘" << std::endl;
}

bool SudokuSolver::solveSudoku() {
    std::cout << "正在生成SAT约束..." << std::endl;
    generateSATClauses();
    
    std::cout << "生成了 " << satSolver.formula.numClauses << " 个子句" << std::endl;
    std::cout << "开始求解..." << std::endl;
    
    bool result = satSolver.solve(true); // 使用优化
    
    if (result) {
        std::cout << "数独求解成功！" << std::endl;
        extractSolution();
        return true;
    } else {
        std::cout << "数独无解！" << std::endl;
        return false;
    }
}

void SudokuSolver::playInteractiveGame() {
    std::cout << "=== 数独游戏 ===" << std::endl;
    std::cout << "选择输入方式：" << std::endl;
    std::cout << "1. 从文件读取" << std::endl;
    std::cout << "2. 手动输入" << std::endl;
    std::cout << "3. 使用示例题目" << std::endl;
    
    int choice;
    std::cout << "请选择 (1-3): ";
    std::cin >> choice;
    
    switch (choice) {
        case 1: {
            std::cout << "请输入文件名: ";
            char filename[256];
            std::cin >> filename;
            loadSudoku(filename);
            break;
        }
        case 2:
            loadSudokuFromInput();
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
                    grid[i][j] = example[i][j];
                }
            }
            break;
        }
        default:
            std::cout << "无效选择，使用示例题目" << std::endl;
            break;
    }
    
    std::cout << "\n原始题目：" << std::endl;
    printGrid();
    
    clock_t start = clock();
    bool solved = solveSudoku();
    clock_t end = clock();
    
    if (solved) {
        std::cout << "\n解答：" << std::endl;
        printGrid();
        
        double time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
        std::cout << "\n求解时间: " << time << " 毫秒" << std::endl;
        
        satSolver.printStatistics();
    }
}