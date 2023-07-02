CC = gcc
CFLAGS = -std=c++17 -O2 -mavx512f -lglfw -framework OpenGL


all: mandelbrot

mandelbrot: mandelbrot.cpp
	$(CC) $(CFLAGS) -o mandelbrot mandelbrot.cpp

clean:
	rm -f mandelbrot mandelbrot.o