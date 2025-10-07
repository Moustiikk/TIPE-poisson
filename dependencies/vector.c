#include "vector.h"
#include <stdio.h>
#include <math.h>


Vec2 init_V2(float x, float y) {
    Vec2 v = {x, y};
    return v;
}


Vec2 add_V2(Vec2 a, Vec2 b) {
    return init_V2(a.x + b.x, a.y + b.y);
}


Vec2 subs_V2(Vec2 a, Vec2 b) {
    return init_V2(a.x - b.x, a.y - b.y);
}


Vec2 mult_V2(Vec2 a, float scalaire) {
    return init_V2(a.x * scalaire, a.y * scalaire);
}

float prod_V2(Vec2 a, Vec2 b) {
    return a.x*b.y+a.y*b.x;
}

Vec2 divide_V2(Vec2 a, float scalaire) {
    inv_scalaire=1/scalaire;
    return init_V2(a.x * inv_scalaire, a.y * inv_scalaire);
}


Vec2 mean_V2(const Vec2* vecList, int n) {
    if (n <= 0) {
        return init_V2(0.0, 0.0);
    }

    Vec2 som = init_V2(0.0, 0.0);
    for (int i = 0; i < n; i++) {
        som = add_V2(som, vecList[i]);
    }

    return mult_V2(som, 1.0/n);
}


float norm_V2(const Vec2* a) {
    return sqrtf(a->x * a->x + a->y * a->y);
}



void print_V2(const Vec2* a) {
    printf("Vec2(%.2f, %.2f)\n", a->x, a->y);
}
