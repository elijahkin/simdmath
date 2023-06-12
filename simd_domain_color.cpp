/*
    @author Elijah Kin
    @version 1.0
    @date 06/11/23
*/

#include "simd_utils.h"

void identity(complex region) {
    for (int i = 0; i < region.size; i += BATCH_SIZE) {
        __m512d REAL = _mm512_load_pd(&region.real[i]);
        __m512d IMAG = _mm512_load_pd(&region.imag[i]);

        _mm512_store_pd(&region.real[i], REAL);
        _mm512_store_pd(&region.imag[i], IMAG);
    }
}

void inverse(complex region) {
    for (int i = 0; i < region.size; i += BATCH_SIZE) {
        __m512d REAL = _mm512_load_pd(&region.real[i]);
        __m512d IMAG = _mm512_load_pd(&region.imag[i]);

        __m512d SABS = _mm512_add_pd(_mm512_mul_pd(REAL, REAL),
                                     _mm512_mul_pd(IMAG, IMAG));

                REAL = _mm512_div_pd(REAL, SABS);
                IMAG = _mm512_div_pd(IMAG, SABS);
        __m512d  NEG = _mm512_set1_pd(-1);
                IMAG = _mm512_mul_pd(IMAG, NEG);

        _mm512_store_pd(&region.real[i], REAL);
        _mm512_store_pd(&region.imag[i], IMAG);
    }
}

// Euler's formula for the win!
void exp(complex region) {
    for (int i = 0; i < region.size; i += BATCH_SIZE) {
        __m512d REAL = _mm512_load_pd(&region.real[i]);
        __m512d IMAG = _mm512_load_pd(&region.imag[i]);

        __m512d  EXP = _mm512_exp_pd(REAL);
        __m512d  COS = _mm512_cos_pd(IMAG);
        __m512d  SIN = _mm512_sin_pd(IMAG);

        _mm512_store_pd(&region.real[i], _mm512_mul_pd(EXP, COS));
        _mm512_store_pd(&region.imag[i], _mm512_mul_pd(EXP, SIN));
    }
}

// https://en.wikipedia.org/wiki/De_Moivre%27s_formula
void pow(complex region, int n) {
    for (int i = 0; i < region.size; i += BATCH_SIZE) {
        __m512d REAL = _mm512_load_pd(&region.real[i]);
        __m512d IMAG = _mm512_load_pd(&region.imag[i]);

        __m512d  ABS = _mm512_sqrt_pd(_mm512_add_pd(
                                      _mm512_mul_pd(REAL, REAL),
                                      _mm512_mul_pd(IMAG, IMAG)));
        __m512d    N = _mm512_set1_pd(n);
        __m512d PABS = _mm512_pow_pd(ABS, N);

        __m512d  ARG = _mm512_atan2_pd(IMAG, REAL);
        __m512d NARG = _mm512_mul_pd(ARG, N);

        __m512d  COS = _mm512_cos_pd(NARG);
        __m512d  SIN = _mm512_sin_pd(NARG);

        _mm512_store_pd(&region.real[i], _mm512_mul_pd(PABS, COS));
        _mm512_store_pd(&region.imag[i], _mm512_mul_pd(PABS, SIN));
    }
}

// I have the SIMD version for primitive functions that
// come up often, but I can also build up more complex
// functions like this instead of writing lots of SIMD code
void essential_singularity(complex region) {
    inverse(region);
    exp(region);
}

// Idea: have primitives return memcopies of region
// https://stackoverflow.com/questions/14047191/overloading-operators-in-typedef-structs-c

void domain_color(void (*f)(complex region), double center_real,
                       double center_imag, double apothem, int n, std::string name) {
    complex region = complex_linspace(center_real, center_imag, apothem, n);
    f(region);
    double * hsl = complex_to_hsl(region);
    uint8_t * rgb = hsl_to_rgb(hsl, region.size);
    std::string filename = make_filename(name, center_real, center_imag, apothem, 0, n);
    save_png(filename.c_str(), rgb, n, n);
}

int main() {
    domain_color(identity, 0, 0, 1, 2048, "identity");
    domain_color(inverse, 0, 0, 3, 2048, "inverse");
    domain_color(exp, 0, 0, 10, 2048, "exp");
    domain_color(essential_singularity, 0, 0, 0.5, 2048, "essential singularity");
    return 0;
}