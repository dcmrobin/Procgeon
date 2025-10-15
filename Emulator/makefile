# Compiler settings
CXX := g++
CXXFLAGS := -Wall -std=c++17 -O2 -I. -I./SDL2/include
LDFLAGS := -L./SDL2/lib
LDLIBS := -lSDL2 -lSDL2_ttf -lSDL2_mixer

# Directories
BUILD_DIR := build
SRC_DIR := .
EXEC := $(BUILD_DIR)/emulator.exe

# DLL paths
SDL_DLL_SOURCE := SDL2/bin/SDL2.dll
SDL_TTF_DLL_SOURCE := SDL2/bin/SDL2_ttf.dll
SDL_MIXER_DLL_SOURCE := SDL2/bin/SDL2_mixer.dll
GCC_DLL_SOURCE := /mingw64/bin/libgcc_s_seh-1.dll
STDCPP_DLL_SOURCE := /mingw64/bin/libstdc++-6.dll
PTHREAD_DLL_SOURCE := /mingw64/bin/libwinpthread-1.dll

# Fonts directory
FONTS_SOURCE := Fonts
FONTS_TARGET := $(BUILD_DIR)/Fonts

# Auto-discover source files
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

# Default target
all: $(EXEC) copy_dll copy_fonts

# Build executable directly from source files
$(EXEC): $(SRCS)
	@echo "Compiling and linking executable directly..."
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Copy necessary DLLs to build directory
copy_dll:
	@echo "Copying DLLs to $(BUILD_DIR)..."
	@cp "$(SDL_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: SDL2.dll not found"
	@cp "$(SDL_TTF_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: SDL2_ttf.dll not found"
	@cp "$(SDL_MIXER_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: SDL2_mixer.dll not found"
	@cp "$(GCC_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: libgcc_s_seh-1.dll not found"
	@cp "$(STDCPP_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: libstdc++-6.dll not found"
	@cp "$(PTHREAD_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: libwinpthread-1.dll not found"

# Copy Fonts folder to build directory
copy_fonts:
	@echo "Copying Fonts folder to $(BUILD_DIR)..."
	@if [ -d "$(FONTS_SOURCE)" ]; then \
		cp -r "$(FONTS_SOURCE)" "$(FONTS_TARGET)" && \
		echo "Fonts copied successfully"; \
	else \
		echo "WARNING: Fonts directory not found"; \
	fi

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

# Run the emulator
run: all
	@echo "Running emulator..."
	cd $(BUILD_DIR) && ./emulator.exe

# Phony targets
.PHONY: all clean run copy_dll copy_fonts