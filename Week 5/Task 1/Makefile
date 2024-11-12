# compiler
CXX = g++
CC = gcc

# flags
CXXFLAGS = -O2 -fopenmp
CFLAGS = -O2 -fopenmp
LDFLAGS = -fopenmp

# sources and objects
CPP_SOURCES = main.cpp lodepng/lodepng.cpp
C_SOURCES = $(wildcard quantize/*.c)
OBJECTS = $(CPP_SOURCES:.cpp=.o) $(C_SOURCES:.c=.o)

# target executable
TARGET = quantpng

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
