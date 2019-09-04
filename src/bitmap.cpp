# include "bitmap.h"

#include <cstdio>

int save_bmp(const char *filename, const void *data, int width, int height) {
    FILE *file = fopen(filename, "wb");
    fwrite(data, 1, width * height * 3, file);
    fclose(file);
    return 0;
}