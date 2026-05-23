CXX         := g++
CXXFLAGS    := -std=c++17 -Wall -Wextra -Iinclude
BIN_DIR     := bin
TARGET      := $(BIN_DIR)/compiler
SRC_DIR     := src
OBJ_DIR     := obj

# The output file path located in the test folder
LEXER_OUTPUT_FILE    := test/output/milestone_1.txt
SYNTAX_OUTPUT_FILE   := test/output/milestone_2.txt
SEMANTIC_OUTPUT_FILE := test/output/milestone_3.txt

# Mencari seluruh file .cpp secara rekursif hingga ke dalam subdirektori
SRCS        := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS        := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Aturan kompilasi dengan pembentukan subdirektori dinamis di dalam obj/
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean removes the output from the test folder and build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LEXER_OUTPUT_FILE) $(SYNTAX_OUTPUT_FILE) $(SEMANTIC_OUTPUT_FILE)

.PHONY: all clean run