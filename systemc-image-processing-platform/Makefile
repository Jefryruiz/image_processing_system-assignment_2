# Makefile for SystemC Image Processing Platform

# SystemC configuration
SYSTEMC_HOME ?= $(shell pkg-config --variable=exec_prefix systemc)
SYSTEMC_CFLAGS := $(shell pkg-config --cflags systemc)
SYSTEMC_LIBS := $(shell pkg-config --libs systemc)

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra $(SYSTEMC_CFLAGS)
LDFLAGS = $(SYSTEMC_LIBS)

# Directories
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = .

# Files
SOURCES = $(SRC_DIR)/main.cpp
HEADERS = $(SRC_DIR)/*.h
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
TARGET = $(BIN_DIR)/image_processor

# Default target
all: $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS)
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
