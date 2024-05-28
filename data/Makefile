# # Compiler settings - Can change to g++ if required
CXX=g++

# # Compiler flags
CXXFLAGS=-Wall -std=c++14 -fopenmp

# # Name of the executable
TARGET=generate_trader
# TARGET=generate_trader

# # All cpp files to be compiled
SRCS=generate_traders.cpp
# SRCS=generate_traders.cpp

# # Default rule to make all
all: $(TARGET)

# # Rule for linking the final executable
# # Depends on the object files (which will be compiled due to implicit rules)
$(TARGET): $(SRCS:.cpp=.o)
	$(CXX) $(CXXFLAGS) -o $@ $^

# # Rule for compiling the source files to object files
# # Uses default implicit rule for compiling .cpp files to .o files

# # Clean rule
clean:
	rm -f $(SRCS:.cpp=.o) $(TARGET)

# # Rule for running the program
run: $(TARGET)
	./$(TARGET)