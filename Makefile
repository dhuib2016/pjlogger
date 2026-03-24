# 编译器及其标志
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
LDFLAGS = -llog4cplus -lpthread

# 目标文件
LIB_NAME = libpjlog.a
LIB_DIR = lib
SRC_DIR = src
INCLUDE_DIR = include
EXAMPLES_DIR = examples
TEST_DIR = test

# 源文件及目标文件
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(LIB_DIR)/%.o, $(SRCS))

# 测试程序
TEST_BENCH = $(TEST_DIR)/benchtest
TEST_THREAD = $(TEST_DIR)/threadtest
TEST_ROOT = $(TEST_DIR)/rootconfig
TEST_BRANCH = $(TEST_DIR)/branchconfig
TEST_THREADCONFIG = $(TEST_DIR)/threadconfig

# 默认目标
all: $(LIB_DIR) $(LIB_DIR)/$(LIB_NAME) $(TEST_BENCH) $(TEST_THREAD) $(TEST_ROOT) $(TEST_BRANCH) $(TEST_THREADCONFIG)

# 创建 build 目录
$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# 编译目标文件
$(LIB_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE_DIR)/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 构建静态库
$(LIB_DIR)/$(LIB_NAME): $(OBJS)
	ar rcs $@ $^

# ========================
# 测试程序
# ========================

# benchtest
$(TEST_BENCH): $(TEST_DIR)/benchtest.cpp $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lpjlog $(LDFLAGS) -o $@

# threadtest
$(TEST_THREAD): $(TEST_DIR)/threadtest.cpp $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lpjlog $(LDFLAGS) -o $@

$(TEST_ROOT): $(TEST_DIR)/rootconfig.cpp $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lpjlog $(LDFLAGS) -o $@

$(TEST_BRANCH): $(TEST_DIR)/branchconfig.cpp $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lpjlog $(LDFLAGS) -o $@

$(TEST_THREADCONFIG): $(TEST_DIR)/threadconfig.cpp $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lpjlog $(LDFLAGS) -o $@

# 清理目标文件和二进制文件
clean:
	rm -rf $(LIB_DIR) $(TEST_BENCH) $(TEST_THREAD) $(TEST_ROOT) $(TEST_BRANCH) $(TEST_THREADCONFIG)

# PHONY 目标
.PHONY: all clean

install:
	@cp ./include/* ../rcl/include/
	@cp $(LIB_DIR)/$(LIB_NAME) ../rcl/lib/
