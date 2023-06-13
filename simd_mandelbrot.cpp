/*
    @author Elijah Kin
    @version 1.2
    @date 06/13/23
*/

#define BATCH_SIZE 8
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <immintrin.h>
#include <iostream>
#include "stb_image_write.h"
#include <thread>
#include <vector>

void simd_mandelbrot_iters(double * real, double * imag,
                           double * escape_iter, int max_iter, int size) {
    for (int i = 0; i < size; i += BATCH_SIZE) {
        // Loading real and imaginary values from memory
        __m512d C_RE = _mm512_load_pd(&real[i]);
        __m512d C_IM = _mm512_load_pd(&imag[i]);
        __m512d Z_RE = _mm512_set1_pd(0);
        __m512d Z_IM = _mm512_set1_pd(0);
        // Defining constant used in the loop below
        __m512d TWO         = _mm512_set1_pd(2);
        __m512d FOUR        = _mm512_set1_pd(4);
        __m512d MAX_ITER    = _mm512_set1_pd(max_iter);
        __m512d ESCAPE_ITER = _mm512_set1_pd(max_iter);
        for (int iter = 0; iter < max_iter; iter++) {
            // Computing the new real values
            __m512d Z_RE_SQUARED = _mm512_mul_pd(Z_RE, Z_RE);
            __m512d Z_IM_SQUARED = _mm512_mul_pd(Z_IM, Z_IM);
            __m512d Z_RE_NEW     = _mm512_sub_pd(Z_RE_SQUARED, Z_IM_SQUARED);
                    Z_RE_NEW     = _mm512_add_pd(Z_RE_NEW, C_RE);
            // Computing the new imaginary values
            __m512d Z_IM_NEW = _mm512_mul_pd(Z_RE, Z_IM);
                    Z_IM_NEW = _mm512_mul_pd(TWO, Z_IM_NEW);
                    Z_IM_NEW = _mm512_add_pd(Z_IM_NEW, C_IM);
            // Checking which points escaped
            __m512d  ABS_SQUARED = _mm512_add_pd(Z_RE_SQUARED, Z_IM_SQUARED);
            __mmask8 ESCAPED     = _mm512_cmp_pd_mask(ABS_SQUARED, FOUR, _CMP_GE_OQ);
            __m512d  THIS_ITER   = _mm512_set1_pd(iter);
            __m512d  BLEND       = _mm512_mask_blend_pd(ESCAPED, MAX_ITER, THIS_ITER);
                     ESCAPE_ITER = _mm512_min_pd(ESCAPE_ITER, BLEND);
            // Setting escaped points to 0 to avoid overflow
            Z_RE_NEW = _mm512_maskz_mov_pd(~ESCAPED, Z_RE_NEW);
            Z_IM_NEW = _mm512_maskz_mov_pd(~ESCAPED, Z_IM_NEW);
            C_RE     = _mm512_maskz_mov_pd(~ESCAPED, C_RE);
            C_IM     = _mm512_maskz_mov_pd(~ESCAPED, C_IM);
            // Updating the points with their new values
            Z_RE = Z_RE_NEW;
            Z_IM = Z_IM_NEW;
        }
        _mm512_store_pd(&escape_iter[i], ESCAPE_ITER);
    }
}

void mandelbrot_worker(double * real, double * imag, uint8_t * rgb, int max_iter, int size) {
    // Allocate memory this worker will need
    double * escape_iter = (double *) malloc(size * sizeof(double));
    // Invoking the SIMD function to do iterations
    simd_mandelbrot_iters(real, imag, escape_iter, max_iter, size);
    // Calculating RGB values from escape iterations
    for (int i = 0; i < size; i++) {
        rgb[3 * i + 0] = 0;
        rgb[3 * i + 1] = (uint8_t) (255 * (escape_iter[i] / max_iter)) + 1;
        rgb[3 * i + 2] = 0;
    }
}

void mandelbrot(double center_real, double center_imag,
                double apothem, int max_iter, int n, int num_threads) {
    auto start = std::chrono::steady_clock::now();
    int size = n * n;
    double * real = (double *) malloc(size * sizeof(double));
    double * imag = (double *) malloc(size * sizeof(double));

    double min_real = center_real - apothem;
    double max_imag = center_imag + apothem;
    double step_size = 2 * apothem / (n - 1);

    for (int i = 0; i < size; i++) {
        real[i] = min_real + (i % n) * step_size;
        imag[i] = max_imag - (i / n) * step_size;
    }

    uint8_t * rgb = (uint8_t *) malloc(3 * size);

    // Setting up the threads
    int thread_size = size / num_threads;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; i++) {
        threads.push_back(std::thread(mandelbrot_worker, &real[i*thread_size],
                    &imag[i*thread_size], &rgb[i*3*thread_size], max_iter, thread_size));
    }
    // Waiting for the threads to finish
    for (std::thread& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    // Saving as a .png image
    char * filename = (char *) malloc(100 * sizeof(char));
    sprintf(filename, "renders/mandelbrot (%.02f, %.02f, %.02f, %i, %i).png",
            center_real, center_imag, apothem, max_iter, n);
    stbi_write_png(filename, n, n, 3, rgb, 0);
    auto end = std::chrono::steady_clock::now();
    std::cout << "Using " << num_threads << " Thread(s): " << std::chrono::duration <double, std::milli> (end - start).count() << " ms" << std::endl;
}

int main() {
    mandelbrot(-0.6, 0, 1.5, 200, 4096, 1);
    mandelbrot(-0.6, 0, 1.5, 200, 4096, 2);
    mandelbrot(-0.6, 0, 1.5, 200, 4096, 4);
    mandelbrot(-0.6, 0, 1.5, 200, 4096, 8);
    mandelbrot(-0.6, 0, 1.5, 200, 4096, 16);
    mandelbrot(-0.6, 0, 1.5, 200, 4096, 32);
    return 0;
}
