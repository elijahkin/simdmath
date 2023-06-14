CC = icc
CFLAGS = -std=c++17 -O2 -mavx512f -framework GLUT -framework OpenGL -diag-disable=10441 -L/usr/local/Cellar/libpng/1.6.39/lib -lpng16

all: mandelbrot

mandelbrot: mandelbrot.cpp
	$(CC) $(CFLAGS) -o bin/mandelbrot mandelbrot.cpp

clean:
	rm -f bin/mandelbrot mandelbrot.o