# Compiler settings
CXX := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -O2 -I. -I./SDL2/include
LDFLAGS := -L./SDL2/lib
LDLIBS := -lSDL2

# Auto-discover source files
SRC_DIR := .
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:%.cpp=%.o)
EXEC := emulator.exe

# Default target
all: $(EXEC)

# Build executable
$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

# Pattern rule for compiling .cpp files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(EXEC) $(OBJS)

# Run the emulator
run: $(EXEC)
	./$(EXEC)

# Phony targets
.PHONY: all clean run