#ifndef __COLOR_H__
#define __COLOR_H__

struct color {
    int value;

    color() {}

    color(int r, int g, int b, int a = 0xff);

    color(int rgb) : value(rgb) {}
};

#endif // __COLOR_H__
