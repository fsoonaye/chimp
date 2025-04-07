# **************************************************************************** #
#   Author: fsoonaye                Makefile                Date: 06/04/2025   #
# **************************************************************************** #

# Specify the name of the output binary
EXE ?= engine

# Compiler and flags
CXX ?= clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -O3 -MMD -MP
LDFLAGS :=

# Architecture-specific optimizations
UNAME_M := $(shell uname -m)

ifeq ($(UNAME_M),arm64)         # macOS on Apple Silicon
	CXXFLAGS += -mcpu=native
else ifeq ($(UNAME_M),aarch64)  # Linux/ARM
	CXXFLAGS += -march=armv8-a
else ifeq ($(UNAME_M),x86_64)   # x64 Intel/AMD
	CXXFLAGS += -march=native
endif


# Directories
SRC_DIR := src
OBJ_DIR := build
DEP_DIR := $(OBJ_DIR)

# File lists
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
HEADERS := $(wildcard $(SRC_DIR)/*.h) $(wildcard $(SRC_DIR)/*.hpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))
DEPENDS := $(patsubst $(OBJ_DIR)/%.o,$(DEP_DIR)/%.d,$(OBJECTS))


# Default target
all: $(EXE)

# Link the final executable
$(EXE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile source files into objects
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Ensure `obj/` exists before compiling
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Include dependency files
-include $(DEPENDS)


# Cleanup targets
clean:
	rm -f $(OBJ_DIR)/*.o $(DEP_DIR)/*.d

fclean: clean
	rm -rf $(OBJ_DIR) $(EXE)

re: fclean all

# Ensure these targets are not treated as files
.PHONY: all clean fclean re
