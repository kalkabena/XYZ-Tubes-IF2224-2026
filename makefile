CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Isrc/include
TARGET   := compiler
SRC_DIR  := src
OBJ_DIR  := obj

SRCS     := $(SRC_DIR)/compiler.cpp 
OBJS     := $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(TARGET)

# The run target: builds the executable then runs it
run: $(TARGET)
	./$(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean run