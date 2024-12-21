# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -pthread

# Directories
OBJ_DIR = build
SRC_DIR = .
BIN_DIR = .

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Object files (place in OBJ_DIR)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o) 

# Executables
JOY_EXEC = galaga

# Default target
all: $(JOY_EXEC)

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Build joy executable
$(JOY_EXEC): $(OBJ_DIR)/client.o $(OBJ_DIR)/keyboard.o $(OBJ_DIR)/galaga.o $(OBJ_DIR)/server.o  | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^
# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR) $(JOY_EXEC)

.PHONY: all clean