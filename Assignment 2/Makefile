CXX = clang++
CC = clang
CXXFLAGS = -std=c++17
CFLAGS = -std=c99
INCLUDES = -I. -Iheaders -Ilib/include
LIBDIRS = -Llib/lib
LIBS = -lassimp -lglfw3 -lz -framework Cocoa -framework IOKit -framework CoreVideo -framework OpenGL

SRC_CPP = src/main.cpp
SRC_C = lib/src/glad.c
OBJ_CPP = build/main.o
OBJ_C = build/glad.o
TARGET = ik_app

all: $(TARGET)

build:
	mkdir -p build

$(OBJ_CPP): $(SRC_CPP) | build
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(SRC_CPP) -o $(OBJ_CPP)

$(OBJ_C): $(SRC_C) | build
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC_C) -o $(OBJ_C)

$(TARGET): $(OBJ_CPP) $(OBJ_C)
	$(CXX) $(CXXFLAGS) $(OBJ_CPP) $(OBJ_C) $(LIBDIRS) $(LIBS) -o $(TARGET)

clean:
	rm -rf build $(TARGET)

.PHONY: all clean build
