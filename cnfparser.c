#include "cnfparser.h"

// ===== CNF解析器创建和销毁 =====
CNFParser* cnfParserCreate() {
    CNFParser* parser = (CNFParser*)malloc(sizeof(CNFParser));
    if (!parser) return NULL;
    
    parser->file = NULL;
    parser->lineNumber = 0;
    parser->hasError = false;
    memset(parser->currentLine, 0, MAX_LINE_LENGTH);
    memset(parser->errorMessage, 0, 256);
    
    return parser;
}

void cnfParserDestroy(CNFParser* parser) {
    if (parser) {
        if (parser->file) {
            fclose(parser->file);
        }
        free(parser);
    }
}

// ===== 文件加载 =====
bool cnfParserLoadFile(CNFParser* parser, const char* filename) {
    if (!parser || !filename) {
        return false;
    }
    
    parser->file = fopen(filename, "r");
    if (!parser->file) {
        snprintf(parser->errorMessage, 256, "无法打开文件: %s", filename);
        parser->hasError = true;
        return false;
    }
    
    parser->lineNumber = 0;
    parser->hasError = false;
    return true;
}

// ===== 主解析函数 =====
CNFFormula* cnfParserParse(CNFParser* parser) {
    if (!parser || !parser->file) {
        cnfParserSetError(parser, "解析器未初始化或文件未打开");
        return NULL;
    }
    
    CNFFormula* formula = cnfFormulaCreate();
    if (!formula) {
        cnfParserSetError(parser, "内存分配失败");
        return NULL;
    }
    
    // 跳过注释行
    if (!cnfParserSkipComments(parser)) {
        cnfFormulaDestroy(formula);
        return NULL;
    }
    
    // 解析头部
    if (!cnfParserParseHeader(parser, formula)) {
        cnfFormulaDestroy(formula);
        return NULL;
    }
    
    // 解析子句
    int clauseId = 1;
    while (fgets(parser->currentLine, MAX_LINE_LENGTH, parser->file)) {
        parser->lineNumber++;
        cnfParserTrimLine(parser->currentLine);
        
        // 跳过空行和注释
        if (parser->currentLine[0] == '\0' || parser->currentLine[0] == 'c') {
            continue;
        }
        
        Clause clause = createClause(clauseId++);
        if (!cnfParserParseClause(parser, &clause, clauseId - 1)) {
            destroyClause(&clause);
            cnfFormulaDestroy(formula);
            return NULL;
        }
        
        if (!clauseIsEmpty(&clause)) {
            cnfFormulaAddClause(formula, clause);
        } else {
            destroyClause(&clause);
        }
    }
    
    // 验证解析结果
    formula->numClauses = formula->clauses.size;
    if (!cnfParserVerifyFormula(formula)) {
        printf("警告：实际子句数量 (%d) 与头部声明不符 (%d)\n", 
               formula->clauses.size, formula->numClauses);
    }
    
    return formula;
}

// ===== 头部解析 =====
bool cnfParserParseHeader(CNFParser* parser, CNFFormula* formula) {
    while (fgets(parser->currentLine, MAX_LINE_LENGTH, parser->file)) {
        parser->lineNumber++;
        cnfParserTrimLine(parser->currentLine);
        
        if (parser->currentLine[0] == 'p') {
            char p[10], cnf[10];
            if (sscanf(parser->currentLine, "%s %s %d %d", 
                      p, cnf, &formula->numVars, &formula->numClauses) != 4) {
                cnfParserSetError(parser, "CNF文件头部格式不正确");
                return false;
            }
            
            if (strcmp(cnf, "cnf") != 0) {
                cnfParserSetError(parser, "不是标准CNF格式文件");
                return false;
            }
            
            if (formula->numVars <= 0 || formula->numClauses < 0) {
                cnfParserSetError(parser, "变量数或子句数无效");
                return false;
            }
            
            return true;
        }
    }
    
    cnfParserSetError(parser, "未找到CNF文件头部");
    return false;
}

// ===== 子句解析 =====
bool cnfParserParseClause(CNFParser* parser, Clause* clause, int clauseId) {
    (void)clauseId; // 避免未使用参数警告
    char* token = strtok(parser->currentLine, " \t\n");
    
    while (token != NULL) {
        int literal = atoi(token);
        
        if (literal == 0) {
            break; // 子句结束
        }
        
        // 基本有效性检查
        if (literal < -MAX_VARIABLES || literal > MAX_VARIABLES) {
            char errorMsg[256];
            snprintf(errorMsg, 256, "第%d行：文字超出范围 %d", parser->lineNumber, literal);
            cnfParserSetError(parser, errorMsg);
            return false;
        }
        
        bool sign = (literal > 0);
        int var = abs(literal);
        
        clauseAddLiteral(clause, createLiteral(var, sign));
        token = strtok(NULL, " \t\n");
    }
    
    return true;
}

// ===== 跳过注释 =====
bool cnfParserSkipComments(CNFParser* parser) {
    long pos = ftell(parser->file);
    
    while (fgets(parser->currentLine, MAX_LINE_LENGTH, parser->file)) {
        parser->lineNumber++;
        cnfParserTrimLine(parser->currentLine);
        
        if (parser->currentLine[0] != 'c' && parser->currentLine[0] != '\0') {
            // 回到非注释行的开始
            fseek(parser->file, pos, SEEK_SET);
            parser->lineNumber--;
            return true;
        }
        pos = ftell(parser->file);
    }
    
    return true;
}

// ===== 验证函数 =====
bool cnfParserVerifyFormula(const CNFFormula* formula) {
    if (!formula) return false;
    
    // 检查子句数量
    if (formula->clauses.size != formula->numClauses) {
        return false;
    }
    
    // 检查变量编号范围
    for (int i = 0; i < formula->clauses.size; i++) {
        const Clause* clause = &formula->clauses.data[i];
        for (int j = 0; j < clause->literals.size; j++) {
            int var = clause->literals.data[j].var;
            if (var < 1 || var > formula->numVars) {
                return false;
            }
        }
    }
    
    return true;
}

// ===== 输出函数 =====
void cnfParserPrintFormula(const CNFFormula* formula) {
    if (!formula) return;
    
    printf("=== CNF公式内容 ===\n");
    printf("p cnf %d %d\n", formula->numVars, formula->numClauses);
    
    for (int i = 0; i < formula->clauses.size; i++) {
        cnfParserPrintClause(&formula->clauses.data[i]);
    }
    
    printf("===================\n");
}

void cnfParserPrintClause(const Clause* clause) {
    if (!clause) return;
    
    for (int i = 0; i < clause->literals.size; i++) {
        if (!clause->literals.data[i].sign) {
            printf("-");
        }
        printf("%d ", clause->literals.data[i].var);
    }
    printf("0\n");
}

// ===== 错误处理 =====
void cnfParserSetError(CNFParser* parser, const char* message) {
    if (parser && message) {
        strncpy(parser->errorMessage, message, 255);
        parser->errorMessage[255] = '\0';
        parser->hasError = true;
    }
}

const char* cnfParserGetError(const CNFParser* parser) {
    return parser ? parser->errorMessage : "解析器为空";
}

bool cnfParserHasError(const CNFParser* parser) {
    return parser ? parser->hasError : true;
}

// ===== 工具函数 =====
bool cnfParserIsValidLiteral(int literal, int maxVar) {
    int var = abs(literal);
    return var >= 1 && var <= maxVar;
}

void cnfParserTrimLine(char* line) {
    if (!line) return;
    
    // 移除末尾的空白字符
    int len = strlen(line);
    while (len > 0 && (line[len-1] == ' ' || line[len-1] == '\t' || 
                       line[len-1] == '\n' || line[len-1] == '\r')) {
        line[--len] = '\0';
    }
    
    // 移除开头的空白字符
    int start = 0;
    while (line[start] == ' ' || line[start] == '\t') {
        start++;
    }
    
    if (start > 0) {
        memmove(line, line + start, len - start + 1);
    }
}