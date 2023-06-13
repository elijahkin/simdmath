CC = icc
CFLAGS = -std=c++17 -O2 -mavx512f -diag-disable=10441 -L/usr/local/Cellar/libpng/1.6.39/lib -lpng16

all: simd_mandelbrot

simd_mandelbrot: simd_mandelbrot.cpp
	$(CC) $(CFLAGS) -o bin/simd_mandelbrot simd_mandelbrot.cpp

clean:
	rm -f bin/simd_mandelbrot simd_mandelbrot.o