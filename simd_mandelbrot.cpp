/*
    @author Elijah Kin
    @version 1.0
    @date 06/11/23
*/

#include "simd_utils.h"

void simd_mandelbrot_iter(complex region, int iters) {
    for (int i = 0; i < region.size; i += BATCH_SIZE) {
        __m512d C_RE = _mm512_load_pd(&region.real[i]);
        __m512d C_IM = _mm512_load_pd(&region.imag[i]);
        __m512d Z_RE = _mm512_set1_pd(0);
        __m512d Z_IM = _mm512_set1_pd(0);
        for (int itr = 0; itr < iters; itr++) {
            // Applying the Mandelbrot iteration: z_{n+1} = (z_{n})^2 + c
            __m512d Z_RE_NEW = _mm512_sub_pd(_mm512_mul_pd(Z_RE, Z_RE),
                                             _mm512_mul_pd(Z_IM, Z_IM));
                    Z_RE_NEW = _mm512_add_pd(Z_RE_NEW, C_RE);
            __m512d Z_IM_NEW = _mm512_mul_pd(Z_RE, Z_IM);
                    Z_IM_NEW = _mm512_mul_pd(_mm512_set1_pd(2), Z_IM_NEW);
                    Z_IM_NEW = _mm512_add_pd(Z_IM_NEW, C_IM);

            __m512d SABS = _mm512_add_pd(_mm512_mul_pd(Z_RE_NEW, Z_RE_NEW),
                                         _mm512_mul_pd(Z_IM_NEW, Z_IM_NEW));
            __m512d FOUR = _mm512_set1_pd(4);
               auto MASK = _mm512_cmplt_pd_mask(SABS, FOUR);

            // Update the point with its new value
            Z_RE = Z_RE_NEW;
            Z_IM = Z_IM_NEW;
        }
        _mm512_store_pd(&region.real[i], Z_RE);
        _mm512_store_pd(&region.imag[i], Z_IM);
    }
}

void mandelbrot(double center_real, double center_imag,
                double apothem, int iters, int n, bool benchmark) {
    std::chrono::steady_clock::time_point start, end;

    // Stage 1: Computing the escape iterations
    start = std::chrono::steady_clock::now();
    complex region = complex_linspace(center_real, center_imag, apothem, n);
    simd_mandelbrot_iter(region, iters);
    end = std::chrono::steady_clock::now();
    if (benchmark) {
        std::cout << "Stage 1: " << std::chrono::duration <double, std::milli> (end - start).count() << " ms" << std::endl;
    }

    // Stage 2: Converting to HSL
    start = std::chrono::steady_clock::now();
    double * hsl = complex_to_hsl(region);
    end = std::chrono::steady_clock::now();
    if (benchmark) {
        std::cout << "Stage 2: " << std::chrono::duration <double, std::milli> (end - start).count() << " ms" << std::endl;
    }

    // Stage 3: Converting to RGB
    start = std::chrono::steady_clock::now();
    uint8_t * rgb = hsl_to_rgb(hsl, region.size);
    end = std::chrono::steady_clock::now();
    if (benchmark) {
        std::cout << "Stage 3: " << std::chrono::duration <double, std::milli> (end - start).count() << " ms" << std::endl;
    }

    // Stage 4: Saving the image
    start = std::chrono::steady_clock::now();
    std::string filename = make_filename("mandelbrot", center_real, center_imag, apothem, iters, n);
    save_png(filename.c_str(), rgb, n, n);
    end = std::chrono::steady_clock::now();
    if (benchmark) {
        std::cout << "Stage 4: " << std::chrono::duration <double, std::milli> (end - start).count() << " ms" << std::endl;
    }
}

int main() {
    mandelbrot(-0.5, 0, 1.5, 100, 8192, true);
    return 0;
}