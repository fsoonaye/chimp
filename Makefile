CXX ?= clang++
CXXFLAGS := -O3 -march=native -std=c++20 -Wall -Wextra
LDFLAGS := 

# Source and header files
SRC := src/main.cpp src/perft.cpp src/search.cpp src/uci.cpp
HEADERS := include/chess.hpp src/engine.h src/types.h src/uci.h

# Default executable name
EXE ?= engine

# Build target
all: $(EXE)

$(EXE): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(EXE) $(LDFLAGS)

# Clean target
clean:
	rm -f $(EXE)