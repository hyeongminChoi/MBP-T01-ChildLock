CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -Werror -std=c17 --coverage -I./src
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 --coverage -I./src
LDFLAGS = --coverage
GTEST_LIBS = -lgtest -lgtest_main -pthread

BUILD_DIR = build
TARGET = $(BUILD_DIR)/unit_tests
SRC_C = $(wildcard src/*.c)
TEST_CPP = $(wildcard test/*.cpp)

C_OBJECTS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_C))
TEST_OBJECTS = $(patsubst test/%.cpp,$(BUILD_DIR)/%.o,$(TEST_CPP))

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: test/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(C_OBJECTS) $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(GTEST_LIBS)

test: $(TARGET)
	$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
	find . -name "*.gcda" -delete
	find . -name "*.gcno" -delete
