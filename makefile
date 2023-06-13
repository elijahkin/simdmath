CC = icc
CFLAGS = -std=c++17 -O2 -mavx512f -diag-disable=10441

all: simd_mandelbrot simd_domain_color

simd_mandelbrot: simd_mandelbrot.cpp
	$(CC) $(CFLAGS) -o bin/simd_mandelbrot simd_mandelbrot.cpp

simd_domain_color: simd_domain_color.cpp
	$(CC) $(CFLAGS) -o bin/simd_domain_color simd_domain_color.cpp

clean:
	rm -f simd_mandelbrot simd_mandelbrot.o
	rm -f simd_domain_color simd_domain_color.o