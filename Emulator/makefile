# Compiler settings
CXX := g++

# Default build type (release)
BUILD_TYPE ?= release

# Use system SDL2 paths (MSYS2 installation)
SDL2_INCLUDE := /mingw64/include/SDL2
SDL2_LIB := /mingw64/lib

# Check what SDL libraries exist in system paths
SDL_MIXER_EXISTS := $(shell [ -f "$(SDL2_INCLUDE)/SDL_mixer.h" ] && echo "YES" || echo "NO")
SDL_TTF_EXISTS := $(shell [ -f "$(SDL2_INCLUDE)/SDL_ttf.h" ] && echo "YES" || echo "NO")

# Conditional flags based on build type and library availability
ifeq ($(BUILD_TYPE), debug)
    CXXFLAGS := -Wall -std=c++17 -g -DDEBUG -I$(SDL2_INCLUDE) -I.
    ifeq ($(SDL_MIXER_EXISTS), YES)
        CXXFLAGS += -DHAS_SDL_MIXER
    endif
    ifeq ($(SDL_TTF_EXISTS), YES)
        CXXFLAGS += -DHAS_SDL_TTF
    endif
    EXEC := build/emulator-debug.exe
else
    CXXFLAGS := -Wall -std=c++17 -O2 -I$(SDL2_INCLUDE) -I.
    ifeq ($(SDL_MIXER_EXISTS), YES)
        CXXFLAGS += -DHAS_SDL_MIXER
    endif
    ifeq ($(SDL_TTF_EXISTS), YES)
        CXXFLAGS += -DHAS_SDL_TTF
    endif
    EXEC := build/Velho.exe
endif

LDFLAGS := -L$(SDL2_LIB)
LDLIBS := -lSDL2
ifeq ($(SDL_MIXER_EXISTS), YES)
    LDLIBS += -lSDL2_mixer
endif
ifeq ($(SDL_TTF_EXISTS), YES)
    LDLIBS += -lSDL2_ttf
endif

# Directories
SRC_DIR := .
BUILD_DIR := build

# Check for icon file
ICON_SOURCE := icon.ico
ICON_EXISTS := $(shell [ -f "$(ICON_SOURCE)" ] && echo "YES" || echo "NO")
ICON_RC := 
ICON_RESOURCE :=
ifeq ($(ICON_EXISTS), YES)
    ICON_RC := icon.rc
    ICON_RESOURCE := $(BUILD_DIR)/icon.res
endif

# DLL paths - using system DLLs from MSYS2
SDL_DLL_SOURCE := /mingw64/bin/SDL2.dll
SDL_TTF_DLL_SOURCE := /mingw64/bin/SDL2_ttf.dll
SDL_MIXER_DLL_SOURCE := /mingw64/bin/SDL2_mixer.dll
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
	@if [ "$(ICON_EXISTS)" = "YES" ]; then \
		echo "Icon file found, creating resource file..."; \
		echo 'IDI_ICON1 ICON "icon.ico"' > icon.rc; \
		windres icon.rc -O coff -o $(BUILD_DIR)/icon.res; \
	fi
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(EXEC) $(LDFLAGS) $(LDLIBS) $(ICON_RESOURCE)
	@$(MAKE) copy_resources

# Debug build
debug:
	@echo "Building debug version..."
	@mkdir -p $(BUILD_DIR)
	@if [ "$(ICON_EXISTS)" = "YES" ]; then \
		echo "Icon file found, creating resource file..."; \
		echo 'IDI_ICON1 ICON "icon.ico"' > icon.rc; \
		windres icon.rc -O coff -o $(BUILD_DIR)/icon.res; \
	fi
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(EXEC) $(LDFLAGS) $(LDLIBS) $(ICON_RESOURCE)
	@$(MAKE) copy_resources

# Combined copy targets
copy_resources: copy_dll copy_fonts copy_audio

# Copy necessary DLLs to build directory from system paths
copy_dll:
	@echo "Copying DLLs to $(BUILD_DIR)..."
	@cp "$(SDL_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: SDL2.dll not found"
	@if [ "$(SDL_MIXER_EXISTS)" = "YES" ]; then \
		cp "$(SDL_MIXER_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: SDL2_mixer.dll not found"; \
		cp "/mingw64/bin/libopusfile-0.dll" "$(BUILD_DIR)/" || echo "WARNING: libopusfile-0.dll not found"; \
		cp "/mingw64/bin/libmpg123-0.dll" "$(BUILD_DIR)/" || echo "WARNING: libmpg123-0.dll not found"; \
		cp "/mingw64/bin/libFLAC.dll" "$(BUILD_DIR)/" || echo "WARNING: libFLAC.dll not found"; \
		cp "/mingw64/bin/libvorbisfile-3.dll" "$(BUILD_DIR)/" || echo "WARNING: libvorbisfile-3.dll not found"; \
		cp "/mingw64/bin/libogg-0.dll" "$(BUILD_DIR)/" || echo "WARNING: libogg-0.dll not found"; \
		cp "/mingw64/bin/libvorbis-0.dll" "$(BUILD_DIR)/" || echo "WARNING: libvorbis-0.dll not found"; \
		cp "/mingw64/bin/libwavpack-1.dll" "$(BUILD_DIR)/" || echo "WARNING: libwavpack-1.dll not found"; \
		cp "/mingw64/bin/libopus-0.dll" "$(BUILD_DIR)/" || echo "WARNING: libopus-0.dll not found"; \
		cp "/mingw64/bin/libxmp.dll" "$(BUILD_DIR)/" || echo "WARNING: libxmp.dll not found"; \
	fi
	@if [ "$(SDL_TTF_EXISTS)" = "YES" ]; then \
		cp "$(SDL_TTF_DLL_SOURCE)" "$(BUILD_DIR)/" || echo "WARNING: SDL2_ttf.dll not found"; \
	fi
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
	@rm -f icon.rc $(BUILD_DIR)/icon.res
	rm -rf $(BUILD_DIR)

# Run the release emulator
run: release
	@echo "Running release emulator..."
	cd $(BUILD_DIR) && ./Velho.exe

# Run the debug emulator
run-debug: debug
	@echo "Running debug emulator..."
	cd $(BUILD_DIR) && ./emulator-debug.exe

# Run debug emulator with gdb
gdb: debug
	@echo "Running debug emulator with gdb..."
	cd $(BUILD_DIR) && gdb ./emulator-debug.exe

# Test SDL2 installation
test-sdl:
	@echo "Testing SDL2 installation..."
	@echo "SDL2 include path: $(SDL2_INCLUDE)"
	@echo "SDL2 lib path: $(SDL2_LIB)"
	@echo "SDL_mixer available: $(SDL_MIXER_EXISTS)"
	@echo "SDL_ttf available: $(SDL_TTF_EXISTS)"
	@if [ -f "$(SDL_DLL_SOURCE)" ]; then \
		echo "SDL2.dll found: YES"; \
	else \
		echo "SDL2.dll found: NO"; \
	fi

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
	@echo "    make test-sdl - Test SDL2 installation"
	@echo ""
	@echo "  Build directory:"
	@echo "    build/"
	@echo ""
	@echo "  Executable names:"
	@echo "    Release: Velho.exe"
	@echo "    Debug:   emulator-debug.exe"
	@echo ""
	@echo "  Resource directories copied:"
	@echo "    - Fonts/ -> build/Fonts/"
	@echo "    - Audio/ -> build/Audio/"
	@echo ""
	@echo "  Icon support:"
	@echo "    - Automatically detects icon.ico if present"
	@echo "    - Current icon status: $(ICON_EXISTS)"
	@echo "    Include: $(SDL2_INCLUDE)"
	@echo "    Library: $(SDL2_LIB)"
	@echo ""
	@echo "  Debug features:"
	@echo "    - Debug builds include -g flag for debug symbols"
	@echo "    - Debug builds define DEBUG macro for debug prints"
	@echo "    - Use #ifdef DEBUG in code for debug-only sections"

# Phony targets
.PHONY: all release debug clean run run-debug gdb help copy_dll copy_fonts copy_audio copy_resources test-sdl