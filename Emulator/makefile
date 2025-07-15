# Compiler settings
CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -O2 -I. -I./SDL2/include
LDFLAGS := -L./SDL2/lib
LDLIBS := -lSDL2

# Directories
BUILD_DIR := build
SRC_DIR := .
EXEC := $(BUILD_DIR)/emulator.exe

# DLL paths
SDL_DLL_SOURCE := SDL2/bin/SDL2.dll
GCC_DLL_SOURCE := /mingw64/bin/libgcc_s_seh-1.dll
STDCPP_DLL_SOURCE := /mingw64/bin/libstdc++-6.dll
PTHREAD_DLL_SOURCE := /mingw64/bin/libwinpthread-1.dll

# Auto-discover source files
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Default target
all: $(EXEC) copy_dll

# Build executable
$(EXEC): $(OBJS)
	@echo "Linking executable..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Copy necessary DLLs to build directory
copy_dll:
	@echo "Copying DLLs to $(BUILD_DIR)..."
	@cp "$(SDL_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: SDL2.dll not found"
	@cp "$(GCC_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: libgcc_s_seh-1.dll not found"
	@cp "$(STDCPP_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: libstdc++-6.dll not found"
	@cp "$(PTHREAD_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: libwinpthread-1.dll not found"

# Pattern rule for compiling .cpp files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

# Run the emulator
run: all
	@echo "Running emulator..."
	cd $(BUILD_DIR) && ./emulator.exe

# Phony targets
.PHONY: all clean run copy_dll
