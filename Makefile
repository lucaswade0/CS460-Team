

CXX := g++
CXXFLAGS := -std=c++11 -O2 -Wall -Wextra

# Main executable target
PARSER_TARGET := main

# Source files for organized version
SRCS := main.cpp CommentRemover.cpp Tokenizer.cpp CSTParser.cpp SymbolTableBuilder.cpp ASTBuilder.cpp Interpreter.cpp
OBJS := $(SRCS:.cpp=.o)

all: $(PARSER_TARGET)

$(PARSER_TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(PARSER_TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(PARSER_TARGET) $(OBJS)



.PHONY: all both clean test