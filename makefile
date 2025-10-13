# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++23 -pthread -Wall -Wextra
# -Wall: Enable all warnings
# -Wextra: Enable extra warnings
# -pthread: Link pthread library for threads

# Targets
TARGETS = server client

# Default target (when you just run 'make')
all: $(TARGETS)

# Server compilation
server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp

# Client compilation  
client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp

# Clean up compiled files
clean:
	rm -f $(TARGETS)

# Install (just creates the binaries)
install: all

# Phony targets (not actual files)
.PHONY: all clean install