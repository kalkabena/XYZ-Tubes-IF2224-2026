CXX         := g++
CXXFLAGS    := -std=c++17 -Wall -Wextra -Iinclude
BIN_DIR     := bin
TARGET      := $(BIN_DIR)/compiler
SRC_DIR     := src
OBJ_DIR     := obj

# The output file path located in the test folder
LEXER_OUTPUT_FILE := test/milestone_1/lexer_output.txt
SYNTAX_OUTPUT_FILE := test/milestone_2/syntax_output.txt



SRCS        := $(wildcard $(SRC_DIR)/*.cpp)
OBJS        := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(TARGET)

run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean now explicitly removes the output from the test folder
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(LEXER_OUTPUT_FILE) $(SYNTAX_OUTPUT_FILE)


.PHONY: all clean run