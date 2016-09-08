#include "color.h"

color::color(int r, int g, int b, int a) {
    value = ((r & 0xff) |
            ((g & 0xff) << 8) |
            ((b & 0xff) << 16) |
            ((a & 0xff) << 24));
}
