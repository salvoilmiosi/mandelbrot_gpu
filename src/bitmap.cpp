# include "bitmap.h"

#include <cstdio>

int save_ppm(const char *filename, const void *data, int width, int height) {
    FILE *file = fopen(filename, "wb");
    size_t n = 0;
    n += fprintf(file, "P6\n# Mandelbrot screenshot\n%d %d\n%d\n", 
                 width, height, 0xFF);
    n += fwrite(data, 1, width * height * 3, file);
    fclose(file);
    return n;
}