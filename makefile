# Compiler and flags
CXX = g++
# Added -Iinclude to tell the compiler where to find .h files
CXXFLAGS = -std=c++23 -pthread -Wall -Wextra -Iinclude

# Directories
SRC_DIR = src
EXE_DIR = exe

# Targets - Binaries will now be placed in the exe/ folder
TARGETS = $(EXE_DIR)/server $(EXE_DIR)/client

# Default target
all: $(EXE_DIR) $(TARGETS)

# Ensure the exe directory exists
$(EXE_DIR):
	mkdir -p $(EXE_DIR)

# Server compilation
# Uses src/server.cpp and outputs to exe/server
$(EXE_DIR)/server: $(SRC_DIR)/server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

# Client compilation
# Uses src/client.cpp and outputs to exe/client
$(EXE_DIR)/client: $(SRC_DIR)/client.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

# Clean up compiled files in the exe directory
clean:
	rm -f $(TARGETS)
	rm -rf $(EXE_DIR)

# Install (just creates the binaries)
install: all

# Phony targets
.PHONY: all clean install
