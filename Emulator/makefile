# Compiler settings
CXX := g++

# Default build type (release)
BUILD_TYPE ?= release

# Conditional flags based on build type
ifeq ($(BUILD_TYPE), debug)
    CXXFLAGS := -Wall -std=c++17 -g -DDEBUG -DSDL_MAIN_HANDLED -I./SDL2/include -I.
    EXEC := build/emulator-debug.exe
else
    CXXFLAGS := -Wall -std=c++17 -O2 -DSDL_MAIN_HANDLED -I./SDL2/include -I.
    EXEC := build/emulator.exe
endif

LDFLAGS := -L./SDL2/lib
LDLIBS := -lSDL2 -lSDL2_ttf -lSDL2_mixer

# Directories
SRC_DIR := .
BUILD_DIR := build

# DLL paths
SDL_DLL_SOURCE := SDL2/bin/SDL2.dll
SDL_TTF_DLL_SOURCE := SDL2/bin/SDL2_ttf.dll
SDL_MIXER_DLL_SOURCE := SDL2/bin/SDL2_mixer.dll
GCC_DLL_SOURCE := /mingw64/bin/libgcc_s_seh-1.dll
STDCPP_DLL_SOURCE := /mingw64/bin/libstdc++-6.dll
PTHREAD_DLL_SOURCE := /mingw64/bin/libwinpthread-1.dll

# Resource directories
FONTS_SOURCE := Fonts
FONTS_TARGET := $(BUILD_DIR)/Fonts
AUDIO_SOURCE := Audio
AUDIO_TARGET := $(BUILD_DIR)/Audio

# Auto-discover source files
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')

# Default target (release build)
all: release

# Release build
release:
	@echo "Building release version..."
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(EXEC) $(LDFLAGS) $(LDLIBS)
	@$(MAKE) copy_resources

# Debug build
debug:
	@echo "Building debug version..."
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(EXEC) $(LDFLAGS) $(LDLIBS)
	@$(MAKE) copy_resources

# Combined copy targets
copy_resources: copy_dll copy_fonts copy_audio

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

# Copy Audio folder to build directory
copy_audio:
	@echo "Copying Audio folder to $(BUILD_DIR)..."
	@if [ -d "$(AUDIO_SOURCE)" ]; then \
		cp -r "$(AUDIO_SOURCE)" "$(AUDIO_TARGET)" && \
		echo "Audio copied successfully"; \
	else \
		echo "WARNING: Audio directory not found"; \
	fi

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILD_DIR)

# Run the release emulator
run: release
	@echo "Running release emulator..."
	cd $(BUILD_DIR) && ./emulator.exe

# Run the debug emulator
run-debug: debug
	@echo "Running debug emulator..."
	cd $(BUILD_DIR) && ./emulator-debug.exe

# Run debug emulator with gdb
gdb: debug
	@echo "Running debug emulator with gdb..."
	cd $(BUILD_DIR) && gdb ./emulator-debug.exe

# Help command - display all available targets
help:
	@echo "Available make targets:"
	@echo ""
	@echo "  Build targets:"
	@echo "    make           - Build release version (same as make release)"
	@echo "    make release   - Build optimized release version"
	@echo "    make debug     - Build debug version with symbols and -DDEBUG"
	@echo ""
	@echo "  Run targets:"
	@echo "    make run       - Build and run release version"
	@echo "    make run-debug - Build and run debug version"
	@echo "    make gdb       - Build debug version and launch with gdb"
	@echo ""
	@echo "  Clean targets:"
	@echo "    make clean       - Clean build directory"
	@echo ""
	@echo "  Utility targets:"
	@echo "    make help     - Show this help message"
	@echo ""
	@echo "  Build directory:"
	@echo "    build/"
	@echo ""
	@echo "  Executable names:"
	@echo "    Release: emulator.exe"
	@echo "    Debug:   emulator-debug.exe"
	@echo ""
	@echo "  Resource directories copied:"
	@echo "    - Fonts/ -> build/Fonts/"
	@echo "    - Audio/ -> build/Audio/"
	@echo ""
	@echo "  Debug features:"
	@echo "    - Debug builds include -g flag for debug symbols"
	@echo "    - Debug builds define DEBUG macro for debug prints"
	@echo "    - Use #ifdef DEBUG in code for debug-only sections"

# Phony targets
.PHONY: all release debug clean run run-debug gdb help copy_dll copy_fonts copy_audio copy_resources