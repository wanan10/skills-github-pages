#ifndef CNFPARSER_H
#define CNFPARSER_H

#include "common.h"

// CNF解析器结构
typedef struct {
    FILE* file;
    char currentLine[MAX_LINE_LENGTH];
    int lineNumber;
    bool hasError;
    char errorMessage[256];
} CNFParser;

// CNF解析器函数声明
CNFParser* cnfParserCreate();
void cnfParserDestroy(CNFParser* parser);

// 核心解析函数
bool cnfParserLoadFile(CNFParser* parser, const char* filename);
CNFFormula* cnfParserParse(CNFParser* parser);

// 内部解析函数
bool cnfParserParseHeader(CNFParser* parser, CNFFormula* formula);
bool cnfParserParseClause(CNFParser* parser, Clause* clause, int clauseId);
bool cnfParserSkipComments(CNFParser* parser);

// 验证和输出函数
bool cnfParserVerifyFormula(const CNFFormula* formula);
void cnfParserPrintFormula(const CNFFormula* formula);
void cnfParserPrintClause(const Clause* clause);

// 错误处理
void cnfParserSetError(CNFParser* parser, const char* message);
const char* cnfParserGetError(const CNFParser* parser);
bool cnfParserHasError(const CNFParser* parser);

// 工具函数
bool cnfParserIsValidLiteral(int literal, int maxVar);
void cnfParserTrimLine(char* line);

#endif // CNFPARSER_H