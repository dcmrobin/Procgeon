# Updated Makefile with build directory
CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -O2 -I. -I./SDL2/include
LDFLAGS := -L./SDL2/lib
LDLIBS := -lSDL2

BUILD_DIR := build
SRC_DIR := .
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
EXEC := $(BUILD_DIR)/emulator.exe

# Create build directory
$(shell mkdir -p $(BUILD_DIR))

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

run: $(EXEC)
	./$(EXEC)

.PHONY: all clean run