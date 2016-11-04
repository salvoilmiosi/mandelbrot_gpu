CFLAGS = -g -Wall --std=c++11
LDFLAGS = -mwindows
LIBS = -lmingw32 -lglfw3 -lglew32 -lopengl32
INCLUDE = include

all: bin/mandelbrot_gpu.exe

bin/mandelbrot_gpu.exe: obj/color.o obj/main.o obj/sources.o
	g++ -o $@ obj/color.o obj/main.o obj/sources.o $(LDFLAGS) $(LIBS)

obj/color.o: src/color.cpp include/color.h
	g++ -c $(CFLAGS) -I $(INCLUDE) -o $@ $<

obj/main.o: src/main.cpp
	g++ -c $(CFLAGS) -I $(INCLUDE) -o $@ $<

obj/sources.o: src/sources.cpp include/sources.h
	g++ -c $(CFLAGS) -I $(INCLUDE) -o $@ $<