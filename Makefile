CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
TARGET = sat_solver
SOURCES = main.cpp sat_solver.cpp sudoku_solver.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# 默认目标
all: $(TARGET)

# 构建可执行文件
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# 编译对象文件
%.o: %.cpp sat_solver.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(OBJECTS) $(TARGET)

# 调试版本
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# 安装（可选）
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# 卸载（可选）
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# 运行测试
test: $(TARGET)
	@echo "运行基本测试..."
	@if [ -f test.cnf ]; then \
		./$(TARGET) -f test.cnf -v; \
	else \
		echo "测试文件 test.cnf 不存在"; \
	fi

# 性能测试
benchmark: $(TARGET)
	@echo "运行性能对比测试..."
	@if [ -f test.cnf ]; then \
		./$(TARGET) -f test.cnf -c; \
	else \
		echo "测试文件 test.cnf 不存在"; \
	fi

.PHONY: all clean debug install uninstall test benchmark