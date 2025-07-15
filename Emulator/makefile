# Compiler settings
CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -O2 -I. -I./SDL2/include
LDFLAGS := -L./SDL2/lib
LDLIBS := -lSDL2

# Directories
BUILD_DIR := build
SRC_DIR := .
SDL_DLL_SOURCE := SDL2/bin/SDL2.dll

# Auto-discover source files
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
EXEC := $(BUILD_DIR)/emulator.exe

# Default target
all: $(EXEC) copy_dll

# Build executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Copy SDL2.dll to build directory
copy_dll:
	@if [ -f "$(SDL_DLL_SOURCE)" ]; then \
		echo "Copying SDL2.dll to $(BUILD_DIR)"; \
		cp "$(SDL_DLL_SOURCE)" "$(BUILD_DIR)/"; \
	else \
		echo "WARNING: SDL2.dll not found at $(SDL_DLL_SOURCE)"; \
	fi

# Pattern rule for compiling .cpp files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Run the emulator
run: all
	cd $(BUILD_DIR) && ./emulator.exe

# Phony targets
.PHONY: all clean run copy_dll