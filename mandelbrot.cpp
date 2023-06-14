/*
    @author Elijah Kin
    @version 1.3
    @date 06/13/23
*/

// Defining the constants we will need
const int WIDTH = 2560;
const int HEIGHT = 1600;
const int NUM_THREADS = 8;
const int BATCH_PIXELS = 8;

// Calculating derived constants
const int PIXELS = WIDTH * HEIGHT;
const int THREAD_PIXELS = PIXELS / NUM_THREADS;
const double ASPECT_RATIO = (double) HEIGHT / WIDTH;

// #include <GLFW/glfw3.h>
#include <immintrin.h>
#include <iostream>
#include <png.h>
#include <thread>
#include <vector>

void simd_iterate_thread(double * real, double * imag, double * escape_iter, int max_iter) {
    for (int i = 0; i < THREAD_PIXELS; i += BATCH_PIXELS) {
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

void iterate_thread(double * real, double * imag, double * escape_iter, int max_iter) {
    double c_real, c_imag, z_real, z_imag;
    double z_real_squared, z_imag_squared, z_real_tmp;
    for (int i = 0; i < THREAD_PIXELS; i++) {
        c_real = real[i];
        c_imag = imag[i];
        z_real = 0;
        z_imag = 0;
        for (int iter = 0; iter < max_iter; iter++) {
            z_real_squared = z_real * z_real;
            z_imag_squared = z_imag * z_imag;
            if (z_real_squared + z_imag_squared > 4) {
                escape_iter[i] = iter;
                break;
            }
            z_real_tmp = z_real_squared - z_imag_squared + c_real;
            z_imag = 2 * z_real * z_imag + c_imag;
            z_real = z_real_tmp;
        }
    }
}

void mandelbrot_thread(double * real, double * imag, uint8_t * rgb, int max_iter) {
    // Allocating the memory this thread will need
    double * escape_iter = (double *) calloc(THREAD_PIXELS, sizeof(double));
    // Performing iterations using SIMD intrisincs on this thread
    simd_iterate_thread(real, imag, escape_iter, max_iter);
    // Calculating RGB values from escape iterations
    for (int i = 0; i < THREAD_PIXELS; i++) {
        rgb[3 * i + 0] = 0;
        rgb[3 * i + 1] = (uint8_t) (255 * (escape_iter[i] / max_iter)) + 1;
        rgb[3 * i + 2] = 0;
    }
    free(escape_iter);
}

void save_png(uint8_t * rgb, char * filename) {
    FILE* fp = fopen(filename, "wb");
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    png_init_io(png, fp);
    png_set_IHDR(
        png,
        info,
        WIDTH,
        HEIGHT,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );
    png_write_info(png, info);
    png_bytep row_pointers [HEIGHT];
    for (int i = 0; i < HEIGHT; i++) {
        row_pointers[i] = &rgb[i * WIDTH * 3];
    }
    png_write_image(png, row_pointers);
    png_write_end(png, NULL);
    png_destroy_write_struct(&png, &info);
    fclose(fp);
}

void mandelbrot(double center_real, double center_imag, double apothem, int max_iter) {
    // Allocating the memory we will need
    double * real = (double *) malloc(PIXELS * sizeof(double));
    double * imag = (double *) malloc(PIXELS * sizeof(double));
    uint8_t * rgb = (uint8_t *) malloc(3 * PIXELS);
    // Computing values we need to initialize real and imag
    double min_real = center_real - apothem;
    double max_imag = center_imag + apothem * ASPECT_RATIO;
    double real_step = 2 * apothem / (WIDTH - 1);
    double imag_step = 2 * apothem / (HEIGHT - 1) * ASPECT_RATIO;
    // Initializing the real and imag values
    int row, col;
    for (int i = 0; i < PIXELS; i++) {
        col = i % WIDTH;
        row = i / WIDTH;
        real[i] = min_real + col * real_step;
        imag[i] = max_imag - row * imag_step;
    }
    // Setting up the threads
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread(mandelbrot_thread, &real[i * THREAD_PIXELS],
                            &imag[i * THREAD_PIXELS], &rgb[i * 3 * THREAD_PIXELS], max_iter));
    }
    // Waiting for the threads to finish
    for (std::thread& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    free(real);
    free(imag);
    // Generating the name for the image file
    char filename[100];
    sprintf(filename, "renders/mandelbrot_%i_%i_%i (%.02f, %.02f, %.02f).png",
            max_iter, WIDTH, HEIGHT, center_real, center_imag, apothem);
    // Saving the RGB data to the image
    save_png(rgb, filename);
    free(rgb);
}

int main() {
    auto start = std::chrono::steady_clock::now();

    // Standard Mandelbrot sanity check
    mandelbrot(-0.6, 0, 2, 200);

    // Plotting some Misiurewicz points
    mandelbrot(-0.77568377, 0.13646737, 0.0000001, 800);
    mandelbrot(0.001643721971153, -0.822467633298876, 0.00000000002, 1600);
    mandelbrot(0.026593792304386393, 0.8095285579867694, 0.00000000001, 200);
    mandelbrot(0.4244, 0.200759, 0.00479616, 300);

    auto end = std::chrono::steady_clock::now();
    std::cout << (end - start).count() / 1000000 << " ms" << std::endl;
    return 0;
}
