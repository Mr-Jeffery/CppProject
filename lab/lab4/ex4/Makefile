
SRC_DIR = ./src
SRC = $(wildcard $(SRC_DIR)/*.cpp)
INCLUDE = -I ./include
OBJ = $(patsubst %.cpp, %.o, $(SRC))

TARGET = fibonacci

CXX = g++
CFLAGS = -Wall -c
CXXFLAGS = $(CFLAGS) -O3

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $(INCLUDE)  $<

.PHONY: clean
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)