# Mini Database Engine Makefile

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
TARGET = CRT
SRC = CRT.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o
	# For Windows
	del /Q $(TARGET).exe 2>NUL

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run