# Makefile for compiling a C++ project with multiple source files

# Compiler settings - Can change to clang++ if desired
CXX = mpic++
CXXFLAGS = -Wall -std=c++11 -fopenmp # -I /shared/software/spack/opt/spack/linux-amzn2-skylake_avx512/gcc-7.3.1/papi-6.0.0.1-jevzvgsbwnpxam5t7obqxl45ztwxwvhi/include -L /shared/software/spack/opt/spack/linux-amzn2-skylake_avx512/gcc-7.3.1/papi-6.0.0.1-jevzvgsbwnpxam5t7obqxl45ztwxwvhi/lib -lpapi

# Linker flags
# LDFLAGS =

# Add header file directories here
INCLUDES = -I.

# Source files to compile
SOURCES = main.cpp trader.cpp stock.cpp dataLoader.cpp portfolio.cpp portfolioUtils.cpp orderUtils.cpp orderBook.cpp order.cpp

# Object files to create
OBJECTS = $(SOURCES:.cpp=.o)

# Name of the executable to create
EXECUTABLE = trading_simulator

# Default target
all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS) -o $@

# Rule for compiling source files to object files
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean target for removing compiled files
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

# Additional dependencies for object files can be added below
