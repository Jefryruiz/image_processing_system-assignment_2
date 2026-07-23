# Makefile for SystemC Image Processing Platform with gem5 Integration

# ============================================================================
# gem5 Configuration (SystemC/TLM-2.0)
# ============================================================================
GEM5_HOME ?= $(HOME)/Documents/gem5
GEM5_ARCH ?= ARM
GEM5_BUILD_DIR = $(GEM5_HOME)/build/$(GEM5_ARCH)
GEM5_SRC = $(GEM5_HOME)/src

# gem5 flags for standalone SystemC (header-only for basic TLM)
# For full gem5 integration, uncomment: GEM5_LIBS = -L$(GEM5_BUILD_DIR) -lgem5_opt
GEM5_CFLAGS = -I$(GEM5_BUILD_DIR) -I$(GEM5_SRC) -I$(GEM5_HOME)/ext/systemc/src
GEM5_LIBS =

# ============================================================================
# SystemC Configuration
# ============================================================================
# Use gem5's built-in SystemC (includes TLM-2.0 and required dependencies)
# instead of system-wide SystemC to ensure compatibility
SYSTEMC_LIB_DIR = $(GEM5_HOME)/util/tlm/build/systemc
SYSTEMC_CFLAGS = -I$(GEM5_HOME)/ext/systemc/src -I$(GEM5_HOME)/src/systemc
SYSTEMC_LIBS = -L$(SYSTEMC_LIB_DIR) -lsystemc

# ============================================================================
# Compiler Settings
# ============================================================================
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -fPIC $(SYSTEMC_CFLAGS) $(GEM5_CFLAGS)
LDFLAGS = $(SYSTEMC_LIBS) $(GEM5_LIBS) -Wl,-rpath,$(SYSTEMC_LIB_DIR)

# ============================================================================
# Build Directories
# ============================================================================
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = .

# ============================================================================
# Files
# ============================================================================
SOURCES = $(SRC_DIR)/main.cpp
HEADERS = $(SRC_DIR)/*.h
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TARGET = $(BIN_DIR)/image_processor

# ============================================================================
# Targets
# ============================================================================

# Default target
all: check-gem5 $(TARGET)

# Check gem5 installation
check-gem5:
	@if [ ! -d "$(GEM5_BUILD_DIR)" ]; then \
		echo "ERROR: gem5 build not found at $(GEM5_BUILD_DIR)"; \
		echo "Set GEM5_HOME to your gem5 installation directory"; \
		exit 1; \
	fi
	@echo "✓ gem5 found at $(GEM5_HOME) ($(GEM5_ARCH) architecture)"

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "✓ Build successful! Executable: $(TARGET)"
	@echo "  Run with: make run"

# Run the simulation standalone (pure SystemC)
run: $(TARGET)
	@echo "Running image processing simulation (standalone SystemC)..."
	LD_LIBRARY_PATH=$(SYSTEMC_LIB_DIR):$(LD_LIBRARY_PATH) ./$(TARGET)

# Run with gem5 (if integrated)
run-with-gem5: $(TARGET)
	@echo "Running with gem5 integration..."
	@echo "See README.gem5.md for gem5 configuration details"

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Show configuration
config:
	@echo "=== Build Configuration ==="
	@echo "gem5 Home:      $(GEM5_HOME)"
	@echo "gem5 Build:     $(GEM5_BUILD_DIR)"
	@echo "gem5 Arch:      $(GEM5_ARCH)"
	@echo "SystemC Lib:    $(SYSTEMC_LIB_DIR)"
	@echo "CXX:            $(CXX)"
	@echo "CXXFLAGS:       $(CXXFLAGS)"
	@echo "LDFLAGS:        $(LDFLAGS)"

# Help
help:
	@echo "=== Image Processing System with gem5 Integration ==="
	@echo ""
	@echo "Available targets:"
	@echo "  make all              - Build the project (default)"
	@echo "  make run              - Build and run as standalone SystemC"
	@echo "  make run-with-gem5    - Run with gem5 integration"
	@echo "  make config           - Show build configuration"
	@echo "  make clean            - Remove build artifacts"
	@echo "  make help             - Show this help message"
	@echo ""
	@echo "Configuration:"
	@echo "  GEM5_HOME=path       - Set gem5 installation (default: ~/Documents/gem5)"
	@echo "  GEM5_ARCH=ARCH       - Set architecture: ARM, X86, ALL (default: ARM)"
	@echo ""
	@echo "Example:"
	@echo "  make clean && make config && make run"

.PHONY: all run run-with-gem5 clean help config check-gem5
