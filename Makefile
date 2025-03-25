CXX = g++
CFLAGS = -Wall
TARGET = test_gates

all: $(TARGET)

$(TARGET): gates.o test_gates.o
	$(CXX) $(CFLAGS) -o $(TARGET) gates.o test_gates.o

gates.o: gates.cpp gates.hpp cacheutils.h
	$(CXX) $(CFLAGS) -c gates.cpp

test_gates.o: test_gates.cpp gates.hpp cacheutils.h
	$(CXX) $(CFLAGS) -c test_gates.cpp

test: $(TARGET)
	taskset -c 8 ./$(TARGET)

clean:
	rm -f *.o $(TARGET)

.PHONY: all test clean