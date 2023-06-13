/*
    @author Elijah Kin
    @version 1.2
    @date 06/13/23
*/

#define BATCH_SIZE 8
#define NUM_THREADS 8

// #include <GLFW/glfw3.h>
#include <immintrin.h>
#include <iostream>
#include <png.h>
#include <thread>
#include <vector>

void iterate_simd(double * real, double * imag, double * escape_iter, int max_iter, int size) {
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
    // Allocating the memory this worker will need
    double * escape_iter = (double *) malloc(size * sizeof(double));
    // Invoking the function to do iterations with SIMD intrisincs
    iterate_simd(real, imag, escape_iter, max_iter, size);
    // Calculating RGB values from escape iterations
    for (int i = 0; i < size; i++) {
        rgb[3 * i + 0] = 0;
        rgb[3 * i + 1] = (uint8_t) (255 * (escape_iter[i] / max_iter)) + 1;
        rgb[3 * i + 2] = 0;
    }
    free(escape_iter);
}

void save_png(const char* filename, uint8_t* rgb, int width, int height) {
    FILE* fp = fopen(filename, "wb");
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    png_init_io(png, fp);
    png_set_IHDR(
        png,
        info,
        width,
        height,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);
    png_bytep row_pointers [height];
    for (int i = 0; i < height; i++) {
        row_pointers[i] = &rgb[i * width * 3];
    }
    png_write_image(png, row_pointers);
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
}

void mandelbrot(double center_real, double center_imag, double apothem, int max_iter, int n) {
    int size = n * n;
    // Allocating the memory we will need
    double * real = (double *) malloc(size * sizeof(double));
    double * imag = (double *) malloc(size * sizeof(double));
    uint8_t * rgb = (uint8_t *) malloc(3 * size);
    // Initiliazing the real and imaginary values
    double min_real = center_real - apothem;
    double max_imag = center_imag + apothem;
    double step_size = 2 * apothem / (n - 1);
    for (int i = 0; i < size; i++) {
        real[i] = min_real + (i % n) * step_size;
        imag[i] = max_imag - (i / n) * step_size;
    }
    // Setting up the threads
    int thread_size = size / NUM_THREADS;
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread(mandelbrot_worker, &real[i*thread_size],
                    &imag[i*thread_size], &rgb[i*3*thread_size], max_iter, thread_size));
    }
    // Waiting for the threads to finish
    for (std::thread& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    free(real);
    free(imag);
    // Saving as a .png image
    char * filename = (char *) malloc(100 * sizeof(char));
    sprintf(filename, "renders/mandelbrot (%.02f, %.02f, %.02f, %i, %i).png",
            center_real, center_imag, apothem, max_iter, n);
    save_png(filename, rgb, n, n);
    free(filename);
    free(rgb);
}

int main() {
    auto start = std::chrono::steady_clock::now();
    mandelbrot(-0.6, 0, 1.5, 255, 8192);
    auto end = std::chrono::steady_clock::now();
    std::cout << (end - start).count() / 1000000 << " ms" << std::endl;
    return 0;
}
