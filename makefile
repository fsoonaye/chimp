CXX := clang++
CXXFLAGS := -Wall -Wextra -Wpedantic -std=c++20 -O2

SRC := src/main.cpp src/search.cpp src/uci.cpp src/evaluate.cpp src/moveorder.cpp src/perft.cpp
OBJ := $(SRC:.cpp=.o)
DEPS := include/chess.hpp src/engine.h src/uci.h src/hash.h src/evaluate.h

TARGET := chimp
BUILD_DIR := bin

# Si une valeur est passée avec make build=xxx, elle est utilisée, sinon c'est chimp
OUTPUT := $(if $(build),$(BUILD_DIR)/$(build),$(TARGET))

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Générer une version spécifique avec make build=chimpv1.2
build: $(OBJ)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $(OUTPUT)

src/%.o: src/%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all build clean