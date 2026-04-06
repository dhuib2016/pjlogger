# �����������־
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
LDFLAGS = -llog4cplus -lpthread

# Ŀ���ļ�
LIB_NAME = libpjlog.a
LIB_DIR = lib
SRC_DIR = src
INCLUDE_DIR = include
EXAMPLES_DIR = examples
TEST_DIR = test

# Դ�ļ���Ŀ���ļ�
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(LIB_DIR)/%.o, $(SRCS))

# ���Գ���
TEST_BENCH = $(TEST_DIR)/benchtest
TEST_THREAD = $(TEST_DIR)/threadtest
TEST_ROOT = $(TEST_DIR)/rootconfig
TEST_BRANCH = $(TEST_DIR)/branchconfig
TEST_THREADCONFIG = $(TEST_DIR)/threadconfig
TEST_LEVELS = $(TEST_DIR)/test_levels
TEST_APPENDERS = $(TEST_DIR)/test_appenders
TEST_NDC = $(TEST_DIR)/test_ndc
TEST_LOGGER_API = $(TEST_DIR)/test_logger_api

# Ĭ��Ŀ��
all: $(LIB_DIR) $(LIB_DIR)/$(LIB_NAME) $(TEST_BENCH) $(TEST_THREAD) $(TEST_ROOT) $(TEST_BRANCH) $(TEST_THREADCONFIG) $(TEST_LEVELS) $(TEST_APPENDERS) $(TEST_NDC) $(TEST_LOGGER_API)

# ���� build Ŀ¼
$(LIB_DIR):
	mkdir -p $(LIB_DIR)

# ����Ŀ���ļ�
$(LIB_DIR)/%.o: $(SRC_DIR)/%.cpp $(INCLUDE_DIR)/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ������̬��
$(LIB_DIR)/$(LIB_NAME): $(OBJS)
	ar rcs $@ $^

# ========================
# ���Գ���
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

$(TEST_LEVELS): $(TEST_DIR)/test_levels.cpp $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lpjlog $(LDFLAGS) -o $@

$(TEST_APPENDERS): $(TEST_DIR)/test_appenders.cpp $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lpjlog $(LDFLAGS) -o $@

$(TEST_NDC): $(TEST_DIR)/test_ndc.cpp $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lpjlog $(LDFLAGS) -o $@

$(TEST_LOGGER_API): $(TEST_DIR)/test_logger_api.cpp $(LIB_DIR)/$(LIB_NAME)
	$(CXX) $(CXXFLAGS) $< -L$(LIB_DIR) -lpjlog $(LDFLAGS) -o $@

# ����Ŀ���ļ��Ͷ������ļ�
clean:
	rm -rf $(LIB_DIR) $(TEST_BENCH) $(TEST_THREAD) $(TEST_ROOT) $(TEST_BRANCH) $(TEST_THREADCONFIG) $(TEST_LEVELS) $(TEST_APPENDERS) $(TEST_NDC) $(TEST_LOGGER_API)

# PHONY Ŀ��
.PHONY: all clean

install:
	@cp ./include/* ../rcl/include/
	@cp $(LIB_DIR)/$(LIB_NAME) ../rcl/lib/
