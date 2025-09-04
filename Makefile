CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -O2 -g
TARGET = sat_solver
SRCDIR = .
OBJDIR = obj

# 模块源文件
COMMON_SRCS = common.c
CNFPARSER_SRCS = cnfparser.c
SOLVER_SRCS = solver.c
XSUDOKU_SRCS = xsudoku.c
DISPLAY_SRCS = display.c
MAIN_SRCS = main.c

# 所有源文件
SOURCES = $(COMMON_SRCS) $(CNFPARSER_SRCS) $(SOLVER_SRCS) $(XSUDOKU_SRCS) $(DISPLAY_SRCS) $(MAIN_SRCS)
OBJECTS = $(SOURCES:.c=.o)
HEADERS = common.h cnfparser.h solver.h xsudoku.h display.h

# 默认目标
all: $(TARGET)

# 构建可执行文件
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) -lm

# 编译对象文件
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# 模块化编译目标
common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o common.o

cnfparser.o: cnfparser.c cnfparser.h common.h
	$(CC) $(CFLAGS) -c cnfparser.c -o cnfparser.o

solver.o: solver.c solver.h common.h
	$(CC) $(CFLAGS) -c solver.c -o solver.o

xsudoku.o: xsudoku.c xsudoku.h common.h solver.h
	$(CC) $(CFLAGS) -c xsudoku.c -o xsudoku.o

display.o: display.c display.h common.h cnfparser.h solver.h xsudoku.h
	$(CC) $(CFLAGS) -c display.c -o display.o

main.o: main.c display.h
	$(CC) $(CFLAGS) -c main.c -o main.o

# 清理生成的文件
clean:
	rm -f $(OBJECTS) $(TARGET)

# 深度清理
distclean: clean
	rm -f *~ core *.core

# 调试版本
debug: CFLAGS += -DDEBUG -O0
debug: clean $(TARGET)

# 发布版本
release: CFLAGS += -DNDEBUG -O3
release: clean $(TARGET)

# 安装
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	chmod 755 /usr/local/bin/$(TARGET)

# 卸载
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# 创建测试文件
test-files:
	@echo "创建测试文件..."
	@echo "c 简单的SAT测试用例" > test_simple.cnf
	@echo "c 可满足的公式: (x1 OR x2) AND (NOT x1 OR x3) AND (NOT x2 OR NOT x3)" >> test_simple.cnf
	@echo "p cnf 3 3" >> test_simple.cnf
	@echo "1 2 0" >> test_simple.cnf
	@echo "-1 3 0" >> test_simple.cnf
	@echo "-2 -3 0" >> test_simple.cnf
	
	@echo "c 不可满足的SAT测试用例" > test_unsat.cnf
	@echo "c 不可满足的公式: x1 AND NOT x1" >> test_unsat.cnf
	@echo "p cnf 1 2" >> test_unsat.cnf
	@echo "1 0" >> test_unsat.cnf
	@echo "-1 0" >> test_unsat.cnf
	
	@echo "c 复杂的SAT测试用例" > test_complex.cnf
	@echo "c 用于测试性能和优化效果" >> test_complex.cnf
	@echo "p cnf 10 20" >> test_complex.cnf
	@echo "1 2 3 0" >> test_complex.cnf
	@echo "-1 4 5 0" >> test_complex.cnf
	@echo "-2 -4 6 0" >> test_complex.cnf
	@echo "-3 -5 -6 0" >> test_complex.cnf
	@echo "7 8 9 0" >> test_complex.cnf
	@echo "-7 -8 10 0" >> test_complex.cnf
	@echo "-9 -10 1 0" >> test_complex.cnf
	@echo "2 -1 -7 0" >> test_complex.cnf
	@echo "3 -2 -8 0" >> test_complex.cnf
	@echo "4 -3 -9 0" >> test_complex.cnf
	@echo "5 -4 -10 0" >> test_complex.cnf
	@echo "6 -5 7 0" >> test_complex.cnf
	@echo "-6 8 -1 0" >> test_complex.cnf
	@echo "9 -2 -7 0" >> test_complex.cnf
	@echo "10 -3 -8 0" >> test_complex.cnf
	@echo "1 -4 -9 0" >> test_complex.cnf
	@echo "-1 -5 -10 0" >> test_complex.cnf
	@echo "2 6 7 0" >> test_complex.cnf
	@echo "-2 -6 -7 0" >> test_complex.cnf
	@echo "3 4 5 0" >> test_complex.cnf
	
	@echo "0" > sudoku_example.txt
	@echo "5 3 0 0 7 0 0 0 0" >> sudoku_example.txt
	@echo "6 0 0 1 9 5 0 0 0" >> sudoku_example.txt
	@echo "0 9 8 0 0 0 0 6 0" >> sudoku_example.txt
	@echo "8 0 0 0 6 0 0 0 3" >> sudoku_example.txt
	@echo "4 0 0 8 0 3 0 0 1" >> sudoku_example.txt
	@echo "7 0 0 0 2 0 0 0 6" >> sudoku_example.txt
	@echo "0 6 0 0 0 0 2 8 0" >> sudoku_example.txt
	@echo "0 0 0 4 1 9 0 0 5" >> sudoku_example.txt
	@echo "0 0 0 0 8 0 0 7 9" >> sudoku_example.txt
	
	@echo "1" > xsudoku_example.txt
	@echo "5 3 0 0 7 0 0 0 0" >> xsudoku_example.txt
	@echo "6 0 0 1 9 5 0 0 0" >> xsudoku_example.txt
	@echo "0 9 8 0 0 0 0 6 0" >> xsudoku_example.txt
	@echo "8 0 0 0 6 0 0 0 3" >> xsudoku_example.txt
	@echo "4 0 0 8 0 3 0 0 1" >> xsudoku_example.txt
	@echo "7 0 0 0 2 0 0 0 6" >> xsudoku_example.txt
	@echo "0 6 0 0 0 0 2 8 0" >> xsudoku_example.txt
	@echo "0 0 0 4 1 9 0 0 5" >> xsudoku_example.txt
	@echo "0 0 0 0 8 0 0 7 9" >> xsudoku_example.txt
	
	@echo "测试文件创建完成！"

# 运行测试
test: $(TARGET) test-files
	@echo "=== 运行基本测试 ==="
	@echo "测试简单可满足实例..."
	@./$(TARGET) -f test_simple.cnf --optimize --verify
	@echo ""
	@echo "测试不可满足实例..."
	@./$(TARGET) -f test_unsat.cnf
	@echo ""
	@echo "测试复杂实例..."
	@./$(TARGET) -f test_complex.cnf --optimize

# 性能测试
benchmark: $(TARGET) test-files
	@echo "=== 运行性能测试 ==="
	@./$(TARGET) -f test_complex.cnf --performance

# 数独测试
test-sudoku: $(TARGET) test-files
	@echo "=== 数独测试 ==="
	@echo "3" | ./$(TARGET) --sudoku

# 帮助信息
help:
	@echo "可用的make目标："
	@echo "  all        - 构建程序（默认）"
	@echo "  clean      - 清理对象文件和可执行文件"
	@echo "  distclean  - 深度清理所有生成文件"
	@echo "  debug      - 构建调试版本"
	@echo "  release    - 构建发布版本"
	@echo "  install    - 安装程序到系统"
	@echo "  uninstall  - 从系统卸载程序"
	@echo "  test-files - 创建测试文件"
	@echo "  test       - 运行基本测试"
	@echo "  benchmark  - 运行性能测试"
	@echo "  test-sudoku- 运行数独测试"
	@echo "  help       - 显示此帮助信息"

# 检查代码风格（如果有工具）
check:
	@if command -v cppcheck >/dev/null 2>&1; then \
		echo "运行代码检查..."; \
		cppcheck --enable=all --std=c99 *.c *.h; \
	else \
		echo "cppcheck未安装，跳过代码检查"; \
	fi

# 模块依赖关系
.PHONY: all clean distclean debug release install uninstall test-files test benchmark test-sudoku help check

# 依赖关系
display.o: common.h cnfparser.h solver.h xsudoku.h
xsudoku.o: common.h solver.h
solver.o: common.h
cnfparser.o: common.h
main.o: display.h