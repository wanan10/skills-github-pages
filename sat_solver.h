#ifndef SAT_SOLVER_H
#define SAT_SOLVER_H

#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cmath>

// 自定义动态数组模板类（不使用STL vector）
template<typename T>
class DynamicArray {
private:
    T* data;
    int capacity;
    int size;
    
    void resize() {
        capacity *= 2;
        T* newData = new T[capacity];
        for (int i = 0; i < size; i++) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
    }
    
public:
    DynamicArray() : data(new T[4]), capacity(4), size(0) {}
    
    // 拷贝构造函数
    DynamicArray(const DynamicArray& other) : capacity(other.capacity), size(other.size) {
        data = new T[capacity];
        for (int i = 0; i < size; i++) {
            data[i] = other.data[i];
        }
    }
    
    // 赋值运算符
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            delete[] data;
            capacity = other.capacity;
            size = other.size;
            data = new T[capacity];
            for (int i = 0; i < size; i++) {
                data[i] = other.data[i];
            }
        }
        return *this;
    }
    
    ~DynamicArray() {
        delete[] data;
    }
    
    void push_back(const T& item) {
        if (size >= capacity) {
            resize();
        }
        data[size++] = item;
    }
    
    T& operator[](int index) {
        return data[index];
    }
    
    const T& operator[](int index) const {
        return data[index];
    }
    
    int getSize() const {
        return size;
    }
    
    void clear() {
        size = 0;
    }
    
    void pop_back() {
        if (size > 0) size--;
    }
    
    bool empty() const {
        return size == 0;
    }
};

// 文字结构
struct Literal {
    int var;      // 变量编号 (1, 2, 3, ...)
    bool sign;    // true表示正文字，false表示负文字
    
    Literal() : var(0), sign(true) {}
    Literal(int v, bool s) : var(v), sign(s) {}
    
    bool operator==(const Literal& other) const {
        return var == other.var && sign == other.sign;
    }
    
    Literal negate() const {
        return Literal(var, !sign);
    }
};

// 子句结构
class Clause {
public:
    DynamicArray<Literal> literals;
    bool satisfied;
    
    Clause() : satisfied(false) {}
    
    void addLiteral(const Literal& lit) {
        literals.push_back(lit);
    }
    
    int size() const {
        return literals.getSize();
    }
    
    bool isEmpty() const {
        return literals.empty();
    }
    
    void print() const {
        for (int i = 0; i < literals.getSize(); i++) {
            if (!literals[i].sign) std::cout << "-";
            std::cout << literals[i].var << " ";
        }
        std::cout << "0" << std::endl;
    }
};

// CNF公式结构
class CNFFormula {
public:
    DynamicArray<Clause> clauses;
    int numVars;
    int numClauses;
    
    CNFFormula() : numVars(0), numClauses(0) {}
    
    void addClause(const Clause& clause) {
        clauses.push_back(clause);
    }
    
    void print() const {
        std::cout << "p cnf " << numVars << " " << numClauses << std::endl;
        for (int i = 0; i < clauses.getSize(); i++) {
            clauses[i].print();
        }
    }
};

// 赋值状态
enum AssignmentState {
    UNASSIGNED = 0,
    TRUE = 1,
    FALSE = -1
};

// 前向声明
class SudokuSolver;

// SAT求解器类
class SATSolver {
    friend class SudokuSolver;
private:
    CNFFormula formula;
    int* assignment;  // 变量赋值数组
    DynamicArray<int> decisionStack;  // 决策栈
    DynamicArray<int> propagationQueue;  // 单元传播队列
    
    // 性能统计
    clock_t startTime;
    clock_t endTime;
    long long decisions;
    long long conflicts;
    long long propagations;
    
    // 优化选项
    bool useHeuristics;
    
    // 内部方法
    bool unitPropagate();
    int selectBranchingVariable();
    int selectBranchingVariableWithHeuristic();
    bool isFormulaUnsatisfied();
    bool isFormulaSatisfied() const;
    void backtrack();
    bool dpll();
    void assignLiteral(int var, bool value);
    void unassignVariable(int var);
    
public:
    SATSolver();
    ~SATSolver();
    
    bool loadCNF(const char* filename);
    void printFormula() const;
    bool solve(bool useOptimization = false);
    void printSolution() const;
    void printStatistics() const;
    double getSolvingTime() const;
    void saveSolution(const char* filename) const;
    
    // 验证功能
    bool verifySolution() const;
    
    // 访问器方法
    int getNumVars() const { return formula.numVars; }
    int getNumClauses() const { return formula.numClauses; }
};

// 数独求解器类
class SudokuSolver {
private:
    int grid[9][9];
    SATSolver satSolver;
    
    void generateSATClauses();
    int getVarIndex(int row, int col, int num) const;
    void extractSolution();
    
public:
    SudokuSolver();
    void loadSudoku(const char* filename);
    void loadSudokuFromInput();
    void printGrid() const;
    bool solveSudoku();
    void playInteractiveGame();
};

#endif // SAT_SOLVER_H