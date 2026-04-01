CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Isrc/include
BIN_DIR  := bin
TARGET   := $(BIN_DIR)/compiler
SRC_DIR  := src
OBJ_DIR  := obj

SRCS     := $(SRC_DIR)/compiler.cpp 
OBJS     := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

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

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) compiler

.PHONY: all clean run