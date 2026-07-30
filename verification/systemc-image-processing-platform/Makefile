# Makefile for SystemC Image Processing Platform

# SystemC configuration
#
# Prefers pkg-config (systemc.pc); if that isn't installed, falls back to
# deriving flags directly from SYSTEMC_HOME, supporting both:
#   - a packaged install: headers in $(SYSTEMC_HOME)/include, libs in
#     $(SYSTEMC_HOME)/lib or $(SYSTEMC_HOME)/lib-linux64
#   - a source-tree build: headers in $(SYSTEMC_HOME)/src, libs in
#     $(SYSTEMC_HOME)/build/src/.libs
HAVE_PKGCONFIG_SYSTEMC := $(shell pkg-config --exists systemc 2>/dev/null && echo yes)

ifeq ($(HAVE_PKGCONFIG_SYSTEMC),yes)
    SYSTEMC_HOME ?= $(shell pkg-config --variable=exec_prefix systemc)
    SYSTEMC_CFLAGS := $(shell pkg-config --cflags systemc)
    SYSTEMC_LIBS := $(shell pkg-config --libs systemc)
else ifneq ($(wildcard $(SYSTEMC_HOME)/include),)
    SYSTEMC_CFLAGS := -I$(SYSTEMC_HOME)/include
    ifneq ($(wildcard $(SYSTEMC_HOME)/lib-linux64),)
        SYSTEMC_LIBS := -L$(SYSTEMC_HOME)/lib-linux64 -lsystemc -Wl,-rpath=$(SYSTEMC_HOME)/lib-linux64
    else
        SYSTEMC_LIBS := -L$(SYSTEMC_HOME)/lib -lsystemc -Wl,-rpath=$(SYSTEMC_HOME)/lib
    endif
else ifneq ($(wildcard $(SYSTEMC_HOME)/src),)
    SYSTEMC_CFLAGS := -I$(SYSTEMC_HOME)/src
    SYSTEMC_LIBS := -L$(SYSTEMC_HOME)/build/src/.libs -lsystemc -Wl,-rpath=$(SYSTEMC_HOME)/build/src/.libs
else
    $(error SYSTEMC_HOME is not set (or invalid) and no systemc.pc was found via pkg-config)
endif

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra $(SYSTEMC_CFLAGS)
LDFLAGS = $(SYSTEMC_LIBS)

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = .
DPI_DIR = ../dpi

# Files
SOURCES = $(SRC_DIR)/main.cpp
HEADERS = $(SRC_DIR)/*.h
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DPI_OBJECTS = $(BUILD_DIR)/ram_model.o
TARGET = $(BIN_DIR)/image_processor

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -I$(DPI_DIR) -c $< -o $@

# Shared DPI-C RAM model (verification/dpi/ram_model.c), also imported by
# the SystemVerilog/UVM testbench under verification/sv_tb/ - see ram_mem.h.
$(BUILD_DIR)/ram_model.o: $(DPI_DIR)/ram_model.c $(DPI_DIR)/ram_model.h | $(BUILD_DIR)
	$(CC) -std=c11 -Wall -O2 -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS) $(DPI_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build successful! Executable: $(TARGET)"

# Run the simulation
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# Help
help:
	@echo "Available targets:"
	@echo "  make all      - Build the project (default)"
	@echo "  make run      - Build and run the simulation"
	@echo "  make clean    - Remove build artifacts"
	@echo "  make help     - Show this help message"

.PHONY: all run clean help
