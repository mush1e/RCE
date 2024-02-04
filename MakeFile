# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall

# Source files directory
SRC_DIR = src

# Object files directory
OBJ_DIR = obj

# Executable name
TARGET = server

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Object files
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Linker flags for SQLite library
LIBS = -lsqlite3

# Rule to build executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Rule to build object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Phony target for cleaning up
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
