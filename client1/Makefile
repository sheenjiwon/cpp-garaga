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

# # Executables
JOY_EXEC = joy
# BUTTON_EXEC = button
CONTROLLER_EXEC = controller
# Default target
all: $(CONTROLLER_EXEC) $(JOY_EXEC)

#  $(BUTTON_EXEC) 

# Create object directory if it doesn't exist
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# # Build joy executable
$(JOY_EXEC): $(OBJ_DIR)/User_gpio.o $(OBJ_DIR)/joy.o | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ 

# # Build button executable
# $(BUTTON_EXEC): $(OBJ_DIR)/User_gpio.o $(OBJ_DIR)/button.o | $(OBJ_DIR)
# 	$(CXX) $(CXXFLAGS) -o $@ $^

# controller executable
$(CONTROLLER_EXEC): $(OBJ_DIR)/User_gpio.o $(OBJ_DIR)/controller.o $(OBJ_DIR)/client.o | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lwiringPi -lwiringPiDev

# Compile source files to object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR) $(CONTROLLER_EXEC) $(JOY_EXEC)

#  $(BUTTON_EXEC) 

.PHONY: all clean