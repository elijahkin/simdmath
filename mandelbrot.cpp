/*
    @author Elijah Kin
    @version 1.5
    @date 08/12/23
*/

#include <immintrin.h>
#include <png.h>

#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

// Defining the constants we will need
const int WIDTH = 2560;
const int HEIGHT = 1600;
const int NUM_THREADS = 8;
const int BATCH_PIXELS = 8;

// Calculating derived constants
const int PIXELS = WIDTH * HEIGHT;
const int THREAD_PIXELS = PIXELS / NUM_THREADS;
const double ASPECT_RATIO = (double)HEIGHT / WIDTH;

void iterate_thread_simd(double *real, double *imag, double *escape_iter,
                         int max_iter) {
  for (int i = 0; i < THREAD_PIXELS; i += BATCH_PIXELS) {
    // Loading real and imaginary values from memory
    __m512d C_REAL = _mm512_load_pd(&real[i]);
    __m512d C_IMAG = _mm512_load_pd(&imag[i]);
    __m512d Z_REAL = _mm512_setzero_pd();
    __m512d Z_IMAG = _mm512_setzero_pd();
    // Defining constant used in the loop below
    __m512d ZERO = _mm512_setzero_pd();
    __m512d TWO = _mm512_set1_pd(2);
    __m512d FOUR = _mm512_set1_pd(4);
    __m512d MAX_ITER = _mm512_set1_pd(max_iter);
    __m512d ESCAPE_ITER = _mm512_set1_pd(max_iter);
    for (int iter = 0; iter < max_iter; iter++) {
      // Computing the new real values
      __m512d Z_REAL_NEW = _mm512_fmsub_pd(
          Z_REAL, Z_REAL, _mm512_fmsub_pd(Z_IMAG, Z_IMAG, C_REAL));
      // Computing the new imaginary values
      __m512d Z_IMAG_NEW = _mm512_fmadd_pd(
          Z_REAL, Z_IMAG, _mm512_fmadd_pd(Z_REAL, Z_IMAG, C_IMAG));
      // Checking which points escaped
      __m512d Z_ABS_SQ = _mm512_fmadd_pd(Z_REAL, Z_REAL, _mm512_mul_pd(Z_IMAG, Z_IMAG));
      __mmask8 ESCAPED = _mm512_cmp_pd_mask(Z_ABS_SQ, FOUR, _CMP_GE_OQ);
      __m512d THIS_ITER = _mm512_set1_pd(iter);
      __m512d BLEND = _mm512_mask_blend_pd(ESCAPED, MAX_ITER, THIS_ITER);
      ESCAPE_ITER = _mm512_min_pd(ESCAPE_ITER, BLEND);
      // Updating points with their new values, keeping escaped points fixed
      C_REAL = _mm512_mask_blend_pd(ESCAPED, C_REAL, Z_REAL);
      C_IMAG = _mm512_mask_blend_pd(ESCAPED, C_IMAG, Z_IMAG);
      Z_REAL = _mm512_mask_blend_pd(ESCAPED, Z_REAL_NEW, ZERO);
      Z_IMAG = _mm512_mask_blend_pd(ESCAPED, Z_IMAG_NEW, ZERO);
    }
    _mm512_store_pd(&escape_iter[i], ESCAPE_ITER);
    _mm512_store_pd(&real[i], C_REAL);
    _mm512_store_pd(&imag[i], C_IMAG);
  }
}

void iterate_thread_naive(double *real, double *imag, double *escape_iter,
                          int max_iter) {
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

void hsl_to_rgb(double *hsl, uint8_t *rgb, int num_pixels) {
  uint8_t sextant;
  double h, s, l;
  double chroma, x, m;
  double r, g, b;

  for (int i = 0; i < num_pixels; i++) {
    h = hsl[3 * i + 0];
    s = hsl[3 * i + 1];
    l = hsl[3 * i + 2];

    chroma = (1 - abs(2 * l - 1)) * s;
    x = chroma * (1 - abs(fmod(h / 60, 2) - 1));
    m = l - (chroma / 2);

    sextant = int(h / 60);
    switch (sextant) {
      case 0:
        r = chroma;
        g = x;
        b = 0;
        break;
      case 1:
        r = x;
        g = chroma;
        b = 0;
        break;
      case 2:
        r = 0;
        g = chroma;
        b = x;
        break;
      case 3:
        r = 0;
        g = x;
        b = chroma;
        break;
      case 4:
        r = x;
        g = 0;
        b = chroma;
        break;
      case 5:
        r = chroma;
        g = 0;
        b = x;
        break;
      default:
        r = g = b = 0;
        break;
    }
    rgb[3 * i + 0] = (uint8_t)((r + m) * 255);
    rgb[3 * i + 1] = (uint8_t)((g + m) * 255);
    rgb[3 * i + 2] = (uint8_t)((b + m) * 255);
  }
}

void create_palette(uint8_t *palette, int max_iter) {
  double *hsl = (double *)malloc(3 * max_iter * sizeof(double));

  for (int i = 0; i < max_iter; i++) {
    hsl[3 * i + 0] = fmod(powf(((double)i / max_iter) * 360, 1.5), 360);
    hsl[3 * i + 1] = 0.5;
    hsl[3 * i + 2] = 0.5;  // (double) i / max_iter;
  }

  hsl_to_rgb(hsl, palette, max_iter);
  free(hsl);
}

double lerp(double a, double b, double t) { return a + t * (b - a); }

void color_lerp(double *real, double *imag, double *escape_iter, uint8_t *rgb,
                uint8_t *palette, int max_iter) {
  double log_zn, nu;
  for (int i = 0; i < THREAD_PIXELS; i++) {
    log_zn = log(real[i] * real[i] + imag[i] * imag[i]) / 2;
    nu = log(log_zn / log(2)) / log(2);
    escape_iter[i] += 1 - nu;
  }
  for (int i = 0; i < THREAD_PIXELS; i++) {
    double iter = escape_iter[i];
    if (iter < max_iter) {
      rgb[3 * i + 0] =
          (uint8_t)lerp(palette[3 * ((int)iter) + 0],
                        palette[3 * ((int)iter + 1) + 0], fmod(iter, 1));
      rgb[3 * i + 1] =
          (uint8_t)lerp(palette[3 * ((int)iter) + 1],
                        palette[3 * ((int)iter + 1) + 1], fmod(iter, 1));
      rgb[3 * i + 2] =
          (uint8_t)lerp(palette[3 * ((int)iter) + 2],
                        palette[3 * ((int)iter + 1) + 2], fmod(iter, 1));
    } else {
      rgb[3 * i + 0] = 0;
      rgb[3 * i + 1] = 0;
      rgb[3 * i + 2] = 0;
    }
  }
}

void mandelbrot_thread(double *real, double *imag, uint8_t *rgb, int max_iter) {
  // Allocating the memory this thread will need
  double *escape_iter = (double *)calloc(THREAD_PIXELS, sizeof(double));
  uint8_t *palette = (uint8_t *)malloc(3 * max_iter);
  // Performing iterations using SIMD intrisincs on this thread
  iterate_thread_simd(real, imag, escape_iter, max_iter);
  // Create the palette that will be used to look up colors for the points
  create_palette(palette, max_iter);
  // Color the points according to their escape iteration
  color_lerp(real, imag, escape_iter, rgb, palette, max_iter);
  free(escape_iter);
  free(palette);
}

void save_png(uint8_t *rgb, char *filename) {
  FILE *fp = fopen(filename, "wb");
  png_structp png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info = png_create_info_struct(png);
  png_init_io(png, fp);
  png_set_IHDR(png, info, WIDTH, HEIGHT, 8, PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);
  png_bytep row_pointers[HEIGHT];
  for (int i = 0; i < HEIGHT; i++) {
    row_pointers[i] = &rgb[i * WIDTH * 3];
  }
  png_write_image(png, row_pointers);
  png_write_end(png, NULL);
  png_destroy_write_struct(&png, &info);
  fclose(fp);
}

void mandelbrot(double center_real, double center_imag, double apothem,
                int max_iter) {
  // Allocating the memory we will need
  double *real = (double *)malloc(PIXELS * sizeof(double));
  double *imag = (double *)malloc(PIXELS * sizeof(double));
  uint8_t *rgb = (uint8_t *)calloc(3 * PIXELS, sizeof(uint8_t));
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
  auto start = std::chrono::steady_clock::now();
  std::vector<std::thread> threads;
  for (int i = 0; i < NUM_THREADS; i++) {
    threads.push_back(std::thread(mandelbrot_thread, &real[i * THREAD_PIXELS],
                                  &imag[i * THREAD_PIXELS],
                                  &rgb[i * 3 * THREAD_PIXELS], max_iter));
  }
  // Waiting for the threads to finish
  for (std::thread &thread : threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
  auto end = std::chrono::steady_clock::now();
  std::cout << (end - start).count() / 1000000 << " ms" << std::endl;
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
  // Standard Mandelbrot sanity check
  mandelbrot(-0.6, 0, 2, 300);

  // Plotting some Misiurewicz points
  mandelbrot(-0.77568377, 0.13646737, 0.0000001, 1000);
  mandelbrot(0.001643721971153, -0.822467633298876, 0.00000000002, 1600);
  mandelbrot(0.026593792304386393, 0.8095285579867694, 0.00000000001, 200);
  mandelbrot(0.4244, 0.200759, 0.00479616, 300);

  // double apothem = 3;
  // for (int i = 0; i < 16080; i++) {
  //   mandelbrot(-0.77568377, 0.13646737, apothem, 800, i);
  //   apothem *= 0.998929882193;
  // }
  return 0;
}
