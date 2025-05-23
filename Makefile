# Compiler settings
CXX = g++
CC = gcc

# Target architecture and optimization flags
ARCH_FLAGS = -march=armv8-a+simd -mtune=cortex-a72
OPTIMIZATION = -O3 -ffast-math -funroll-loops

# Warning flags
WARNINGS = -Wall -Wextra -Wpedantic -Wno-unused-parameter

# Debug flags (used in debug build)
DEBUG_FLAGS = -g -DDEBUG -O0

# Standard and feature flags
STD_FLAGS = -std=c++17

# Include directories
INCLUDES = -I.

# All compiler flags
CXXFLAGS = $(STD_FLAGS) $(ARCH_FLAGS) $(OPTIMIZATION) $(WARNINGS) $(INCLUDES)
DEBUG_CXXFLAGS = $(STD_FLAGS) $(ARCH_FLAGS) $(DEBUG_FLAGS) $(WARNINGS) $(INCLUDES)

# Linker flags
LDFLAGS = 
LIBS = -lm

# Directories
SRC_DIR = .
BUILD_DIR = build
DEBUG_DIR = debug

# Source files
SOURCES = main.cpp
HEADERS = obj_detection_util.h

# Object files
OBJECTS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
DEBUG_OBJECTS = $(SOURCES:%.cpp=$(DEBUG_DIR)/%.o)

# Target executable
TARGET = obj_detection_util
DEBUG_TARGET = obj_detection_util_debug

# Default target
.PHONY: all
all: $(TARGET)

# Release build
$(TARGET): $(BUILD_DIR) $(OBJECTS)
	@echo "Linking release build..."
	$(CXX) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $(TARGET)
	@echo "Release build complete: $(TARGET)"

# Debug build
.PHONY: debug
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_DIR) $(DEBUG_OBJECTS)
	@echo "Linking debug build..."
	$(CXX) $(DEBUG_OBJECTS) $(LDFLAGS) $(LIBS) -o $(DEBUG_TARGET)
	@echo "Debug build complete: $(DEBUG_TARGET)"

# Create build directories
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(DEBUG_DIR):
	@mkdir -p $(DEBUG_DIR)

# Compile source files for release
$(BUILD_DIR)/%.o: %.cpp $(HEADERS)
	@echo "Compiling $< (release)..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile source files for debug
$(DEBUG_DIR)/%.o: %.cpp $(HEADERS)
	@echo "Compiling $< (debug)..."
	$(CXX) $(DEBUG_CXXFLAGS) -c $< -o $@

# Performance optimized build
.PHONY: performance
performance: OPTIMIZATION = -O3 -ffast-math -funroll-loops -flto -march=native
performance: LDFLAGS += -flto
performance: $(TARGET)
	@echo "Performance optimized build complete"

# Build for specific ARM processors
.PHONY: cortex-a53
cortex-a53: ARCH_FLAGS = -march=armv8-a+simd -mtune=cortex-a53
cortex-a53: $(TARGET)

.PHONY: cortex-a72
cortex-a72: ARCH_FLAGS = -march=armv8-a+simd -mtune=cortex-a72
cortex-a72: $(TARGET)

.PHONY: cortex-a76
cortex-a76: ARCH_FLAGS = -march=armv8.2-a+simd -mtune=cortex-a76
cortex-a76: $(TARGET)

.PHONY: cortex-a78
cortex-a78: ARCH_FLAGS = -march=armv8.2-a+simd -mtune=cortex-a78
cortex-a78: $(TARGET)

.PHONY: local-mac
local-mac: ARCH_FLAGS = -march=armv8-a+simd -mcpu=native
local-mac: $(TARGET)

# Assembly output for optimization analysis
.PHONY: assembly
assembly: $(BUILD_DIR)
	@echo "Generating assembly output..."
	$(CXX) $(CXXFLAGS) -S signal_processing.cpp -o $(BUILD_DIR)/signal_processing.s
	$(CXX) $(CXXFLAGS) -S main.cpp -o $(BUILD_DIR)/main.s
	@echo "Assembly files generated in $(BUILD_DIR)/"

# Benchmark build with timing
.PHONY: benchmark
benchmark: CXXFLAGS += -DBENCHMARK_MODE
benchmark: $(TARGET)
	@echo "Benchmark build complete"

# Static analysis
.PHONY: analyze
analyze:
	@echo "Running static analysis..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c++17 $(SOURCES) $(HEADERS); \
	else \
		echo "cppcheck not found, skipping static analysis"; \
	fi

# Format code
.PHONY: format
format:
	@echo "Formatting code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i $(SOURCES) $(HEADERS); \
		echo "Code formatted"; \
	else \
		echo "clang-format not found, skipping formatting"; \
	fi

# Run the program
.PHONY: run
run: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

# Run debug version
.PHONY: run-debug
run-debug: $(DEBUG_TARGET)
	@echo "Running $(DEBUG_TARGET)..."
	./$(DEBUG_TARGET)

# Profile with gprof (if available)
.PHONY: profile
profile: CXXFLAGS += -pg
profile: LDFLAGS += -pg
profile: $(TARGET)
	@echo "Profiling build complete. Run './$(TARGET)' then 'gprof $(TARGET) gmon.out'"

# Memory check with valgrind (if available)
.PHONY: memcheck
memcheck: $(DEBUG_TARGET)
	@echo "Running memory check..."
	@if command -v valgrind >/dev/null 2>&1; then \
		valgrind --leak-check=full --show-leak-kinds=all ./$(DEBUG_TARGET); \
	else \
		echo "valgrind not found, skipping memory check"; \
	fi

# Clean build files
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD_DIR) $(DEBUG_DIR)
	rm -f $(TARGET) $(DEBUG_TARGET)
	rm -f gmon.out *.gcov *.gcda *.gcno
	@echo "Clean complete"

# Clean and rebuild
.PHONY: rebuild
rebuild: clean all

# Install (copy to /usr/local/bin)
.PHONY: install
install: $(TARGET)
	@echo "Installing $(TARGET)..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Installation complete"

# Uninstall
.PHONY: uninstall
uninstall:
	@echo "Uninstalling $(TARGET)..."
	sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstall complete"

# Show compilation database for IDE integration
.PHONY: compile-commands
compile-commands:
	@echo "Generating compile_commands.json..."
	@echo '[' > compile_commands.json
	@for src in $(SOURCES); do \
		echo '  {' >> compile_commands.json; \
		echo '    "directory": "'$(PWD)'",' >> compile_commands.json; \
		echo '    "command": "$(CXX) $(CXXFLAGS) -c '$$src'",' >> compile_commands.json; \
		echo '    "file": "'$$src'"' >> compile_commands.json; \
		echo '  },' >> compile_commands.json; \
	done
	@sed -i '$$ s/,$$//' compile_commands.json
	@echo ']' >> compile_commands.json
	@echo "compile_commands.json generated"

# Help target
.PHONY: help
help:
	@echo "ARM NEON Signal Processing Makefile"
	@echo "==================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build release version (default)"
	@echo "  debug        - Build debug version with debugging symbols"
	@echo "  performance  - Build with maximum performance optimizations"
	@echo "  cortex-a53   - Build optimized for Cortex-A53"
	@echo "  cortex-a72   - Build optimized for Cortex-A72"
	@echo "  cortex-a76   - Build optimized for Cortex-A76"
	@echo "  assembly     - Generate assembly output for analysis"
	@echo "  benchmark    - Build with benchmark timing enabled"
	@echo "  analyze      - Run static analysis (requires cppcheck)"
	@echo "  format       - Format code (requires clang-format)"
	@echo "  run          - Build and run release version"
	@echo "  run-debug    - Build and run debug version"
	@echo "  profile      - Build with profiling support"
	@echo "  memcheck     - Run memory check (requires valgrind)"
	@echo "  clean        - Remove all build files"
	@echo "  rebuild      - Clean and rebuild"
	@echo "  install      - Install to /usr/local/bin"
	@echo "  uninstall    - Remove from /usr/local/bin"
	@echo "  compile-commands - Generate compile_commands.json for IDEs"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build release version"
	@echo "  make debug              # Build debug version"
	@echo "  make run                # Build and run"
	@echo "  make cortex-a72         # Build for Cortex-A72"
	@echo "  make clean && make      # Clean rebuild"

# Dependency tracking
-include $(OBJECTS:.o=.d)
-include $(DEBUG_OBJECTS:.o=.d)

# Generate dependency files
$(BUILD_DIR)/%.d: %.cpp
	@mkdir -p $(BUILD_DIR)
	@$(CXX) $(CXXFLAGS) -MM -MT $(BUILD_DIR)/$*.o $< > $@

$(DEBUG_DIR)/%.d: %.cpp
	@mkdir -p $(DEBUG_DIR)
	@$(CXX) $(DEBUG_CXXFLAGS) -MM -MT $(DEBUG_DIR)/$*.o $< > $@