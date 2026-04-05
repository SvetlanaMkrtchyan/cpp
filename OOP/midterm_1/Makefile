CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRCS = main.cpp instructions.cpp lexer.cpp parser.cpp compiler.cpp executor.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = compiler

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
