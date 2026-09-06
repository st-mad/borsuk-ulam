# Compiler and flags
CXX      := g++
CXXFLAGS := -Wall -Wextra -O2 -std=c++17

# Target executable name
TARGET   := game

# Build directory (keeps your project folder clean)
BUILD_DIR := build

# Find all .cpp files in the current folder
SRCS     := $(wildcard *.cpp)
OBJS     := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)

# Raylib linker flags for Linux
# (Requires X11, OpenGL, and system thread/math libraries)
LDFLAGS  := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# --- Rules ---

# Default target: build and link the executable
all: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking target: $@"
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile source files into object files
$(BUILD_DIR)/%.o: %.cpp
	@echo "Compiling: $<"
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run target: Compiles, links, and executes immediately
run: all
	@echo "Launching $(TARGET)..."
	./$(TARGET)

# Clean target: Removes temporary build files and executable
clean:
	@echo "Cleaning project..."
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all run clean
