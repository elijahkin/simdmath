/*
    @author Elijah Kin
    @version 1.0
    @date 06/11/23
*/

#define BATCH_SIZE 8

#include <cmath>
#include <immintrin.h>
#include <iostream>
#include <png.h>

struct complex {
    double * real;
    double * imag;
    int size;
};

complex complex_linspace(double center_real, double center_imag,
                         double apothem, int n) {
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
    return {real, imag, size};
}

double * complex_to_hsl(complex region) {
    static double * hsl;
    hsl = (double *) malloc(3 * region.size * sizeof(double));

    for (int i = 0; i < region.size; i++) {
        hsl[3 * i + 0] = 0.5 * (atan2(region.imag[i], region.real[i]) / M_PI + 1);
        hsl[3 * i + 1] = 1;
        hsl[3 * i + 2] = 2 * atan(sqrt(region.real[i] * region.real[i]
                                       + region.imag[i] * region.imag[i])) / M_PI;
    }
    return hsl;
}

uint8_t * hsl_to_rgb(double * hsl, int size) {
    uint8_t * rgb;
    rgb = (uint8_t *) malloc(3 * size);

    uint8_t sextant;
    double h, s, l;
    double chroma, x, m;
    double r, g, b;

    for (int i = 0; i < size; i++) {
        h = hsl[3 * i + 0];
        s = hsl[3 * i + 1];
        l = hsl[3 * i + 2];

        chroma = (1 - abs(2 * l - 1)) * s;
        x = chroma * (1 - abs(fmod(6 * h, 2) - 1));
        m = l - (chroma / 2);

        sextant = int(h * 6);
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
        rgb[3 * i + 0] = (uint8_t) ((r + m) * 255);
        rgb[3 * i + 1] = (uint8_t) ((g + m) * 255);
        rgb[3 * i + 2] = (uint8_t) ((b + m) * 255);
    }
    return rgb;
}

std::string make_filename(std::string type, double center_real, double center_imag,
                          double apothem, int iters, int n) {
    char filename [100];
    sprintf(filename, "renders/%s (%.02f, %.02f, %.02f, %i, %i).png",
            type.c_str(), center_real, center_imag, apothem, iters, n);
    return filename;
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