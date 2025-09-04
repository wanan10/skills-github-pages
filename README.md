# SAT求解器 - 模块化C语言实现

基于DPLL算法的布尔可满足性问题求解器，采用模块化设计，支持CNF格式求解和X数独游戏。

## 项目特性

### 🏗️ 模块化架构
- **主控、交互与显示模块 (display)**: 用户界面和程序控制
- **CNF解析模块 (cnfparser)**: DIMACS CNF格式文件解析
- **核心DPLL模块 (solver)**: SAT求解算法实现
- **百分号数独模块 (xsudoku)**: 数独游戏生成、归约、求解

### ⚡ 核心功能
- **完整DPLL算法**: 单元传播、回溯搜索、冲突检测
- **多种启发式策略**: VSIDS、随机选择、首个未赋值
- **性能优化**: 高效数据结构和算法优化
- **X数独支持**: 支持对角线约束的数独变体
- **交互式界面**: 友好的用户交互和结果展示

## 快速开始

### 编译项目
```bash
make all
```

### 基本使用
```bash
# SAT问题求解
./sat_solver -f test_simple.cnf --optimize --verify

# 性能对比测试
./sat_solver -f test_complex.cnf --performance

# X数独游戏
./sat_solver --sudoku

# 交互模式
./sat_solver --interactive
```

## 模块详解

### 1. 主控、交互与显示模块 (display)
**文件**: `display.h`, `display.c`

**功能**:
- 命令行参数解析和程序配置
- 多种运行模式管理（SAT求解、数独游戏、性能测试）
- 用户交互界面和菜单系统
- 结果格式化输出和统计信息展示
- 错误处理和状态管理

**核心结构**:
```c
typedef struct {
    char* inputFile;
    char* outputFile;
    bool useOptimization;
    bool verifyResult;
    HeuristicType heuristic;
    ProgramMode mode;
} ProgramConfig;

typedef struct {
    ProgramConfig config;
    CNFParser* parser;
    SATSolver* satSolver;
    SudokuSolver* sudokuSolver;
} MainController;
```

### 2. CNF解析模块 (cnfparser)
**文件**: `cnfparser.h`, `cnfparser.c`

**功能**:
- 标准DIMACS CNF格式文件解析
- 语法验证和错误检测
- 公式内部表示构建
- 解析结果验证和输出

**核心功能**:
```c
CNFParser* cnfParserCreate();
bool cnfParserLoadFile(CNFParser* parser, const char* filename);
CNFFormula* cnfParserParse(CNFParser* parser);
bool cnfParserVerifyFormula(const CNFFormula* formula);
```

### 3. 核心DPLL模块 (solver)
**文件**: `solver.h`, `solver.c`

**功能**:
- 完整DPLL算法实现
- 多种变量选择启发式
- 单元传播和冲突检测
- 性能统计和时间测量

**算法流程**:
1. **单元传播**: 识别并传播单元子句
2. **变量选择**: 使用启发式选择分支变量
3. **递归搜索**: 尝试true/false两种赋值
4. **冲突处理**: 检测冲突并进行回溯

**启发式策略**:
- **VSIDS**: 选择在最多未满足子句中出现的变量
- **First Unassigned**: 选择第一个未赋值变量
- **Random**: 随机选择未赋值变量

### 4. 百分号数独模块 (xsudoku)
**文件**: `xsudoku.h`, `xsudoku.c`

**功能**:
- 标准数独和X数独（对角线约束）支持
- 数独到SAT问题转换
- 约束生成和求解
- 交互式游戏界面

**SAT约束类型**:
1. **格子约束**: 每个格子有且仅有一个数字
2. **行约束**: 每行每个数字恰好出现一次
3. **列约束**: 每列每个数字恰好出现一次
4. **块约束**: 每个3×3块每个数字恰好出现一次
5. **X约束**: 对角线每个数字恰好出现一次（X数独）

## 数据结构设计

### 核心数据结构
```c
// 文字结构
typedef struct {
    int var;      // 变量编号
    bool sign;    // 符号（true=正文字，false=负文字）
} Literal;

// 子句结构
typedef struct {
    LiteralArray literals;
    bool satisfied;
    int id;
} Clause;

// CNF公式结构
typedef struct {
    ClauseArray clauses;
    int numVars;
    int numClauses;
    char* filename;
} CNFFormula;

// SAT求解器结构
typedef struct {
    CNFFormula* formula;
    int* assignment;
    IntArray* decisionStack;
    IntArray* propagationQueue;
    SolverStats stats;
    bool useHeuristics;
} SATSolver;
```

### 自定义动态数组
为避免依赖STL，项目实现了类型安全的动态数组：
- `IntArray`: 整数动态数组
- `LiteralArray`: 文字动态数组  
- `ClauseArray`: 子句动态数组

## 使用示例

### 命令行选项
```bash
./sat_solver [选项]

选项:
  -f <文件>          指定CNF输入文件
  -o <文件>          指定输出文件
  --optimize         使用优化策略
  --verify           验证求解结果
  --show-formula     显示CNF公式
  --quiet            静默模式
  --sudoku           数独游戏模式
  --performance      性能测试模式
  --interactive      交互模式（默认）
  --heuristic <类型> 启发式策略 (none|first|vsids|random)
  --help, -h         显示帮助信息
```

### 性能测试结果
```
测试文件: test_complex.cnf
变量数: 10
子句数: 20

基础DPLL求解...
结果: SAT
时间: 0.004 毫秒
决策次数: 2

优化DPLL求解...  
结果: SAT
时间: 0.002 毫秒
决策次数: 2

优化率: 50.0%
优化效果: 提升 50.0%
```

### 数独求解示例
```
原始题目：
┌─────────┬─────────┬─────────┐
│ 5 3 . │ . 7 . │ . . . │
│ 6 . . │ 1 9 5 │ . . . │
│ . 9 8 │ . . . │ . 6 . │
├─────────┼─────────┼─────────┤
│ 8 . . │ . 6 . │ . . 3 │
│ 4 . . │ 8 . 3 │ . . 1 │
│ 7 . . │ . 2 . │ . . 6 │
├─────────┼─────────┼─────────┤
│ . 6 . │ . . . │ 2 8 . │
│ . . . │ 4 1 9 │ . . 5 │
│ . . . │ . 8 . │ . 7 9 │
└─────────┴─────────┴─────────┘

生成了 12018 个子句
求解时间: 0.235 毫秒
决策次数: 0 (纯单元传播求解)
```

## 构建和测试

### Makefile目标
```bash
make all        # 构建程序
make clean      # 清理文件
make debug      # 调试版本
make release    # 发布版本
make test-files # 创建测试文件
make test       # 运行基本测试
make benchmark  # 性能测试
make help       # 显示帮助
```

### 测试文件
- `test_simple.cnf`: 简单可满足实例
- `test_unsat.cnf`: 不可满足实例
- `test_complex.cnf`: 复杂性能测试实例
- `sudoku_example.txt`: 标准数独示例
- `xsudoku_example.txt`: X数独示例

## 技术特点

### 算法优化
- **VSIDS启发式**: 动态变量选择策略
- **单元传播优化**: 高效的传播队列管理
- **内存优化**: 紧凑的数据结构设计
- **时间复杂度**: 最坏情况指数级，实际问题通常多项式时间

### 代码质量
- **模块化设计**: 清晰的模块划分和接口
- **内存安全**: 严格的内存分配和释放管理
- **错误处理**: 完善的错误检测和报告机制
- **可扩展性**: 易于添加新的启发式和功能

### 性能表现
- **高效求解**: 优化策略显著提升性能
- **内存占用**: 最小化内存使用
- **实时响应**: 快速的用户交互响应

## 扩展功能

### 已实现
- ✅ 完整DPLL算法
- ✅ 多种启发式策略
- ✅ X数独支持
- ✅ 性能对比测试
- ✅ 交互式界面

### 可扩展
- 🔄 CDCL算法实现
- 🔄 预处理技术
- 🔄 并行化求解
- 🔄 更多数独变体
- 🔄 图形界面

## 文件结构

```
├── common.h/.c          # 通用数据结构和工具函数
├── cnfparser.h/.c       # CNF解析模块
├── solver.h/.c          # 核心DPLL求解模块
├── xsudoku.h/.c         # X数独模块
├── display.h/.c         # 主控、交互与显示模块
├── main.c               # 主程序入口
├── Makefile             # 构建脚本
├── README.md            # 项目文档
└── test_*.cnf           # 测试文件
```

## 作者信息

基于DPLL算法的模块化SAT求解器，采用纯C语言实现，适用于教学、研究和实际应用。

**编译环境**: C99标准，GCC编译器  
**运行平台**: Linux/Unix系统  
**许可协议**: 开源项目，遵循相应开源协议