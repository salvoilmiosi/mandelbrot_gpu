CXX = g++
LD = g++
CFLAGS = -g -Wall --std=c++11
LDFLAGS =
LIBS = -lmingw32 -lglfw3 -lglew32 -lopengl32
INCLUDE = include
OUT_BIN = mandelbrot_gpu

ifeq ($(OS),Windows_NT)
	OUT_BIN := $(OUT_BIN).exe
	LDFLAGS += -mwindows
endif

all: bin/$(OUT_BIN)

bin/$(OUT_BIN): obj/color.o obj/main.o obj/sources.o
	g++ -o bin/$(OUT_BIN) obj/color.o obj/main.o obj/sources.o $(LDFLAGS) $(LIBS)

obj/color.o: src/color.cpp include/color.h
	g++ -c $(CFLAGS) -I $(INCLUDE) -o $@ $<

obj/main.o: src/main.cpp
	g++ -c $(CFLAGS) -I $(INCLUDE) -o $@ $<

obj/sources.o: src/sources.cpp include/sources.h
	g++ -c $(CFLAGS) -I $(INCLUDE) -o $@ $<