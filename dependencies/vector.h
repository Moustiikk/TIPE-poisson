#ifndef VECTOR_H
#define VECTOR_H

typedef struct {
    float x, y;
} Vec2;

Vec2 init_V2(float x, float y);
Vec2 add_V2(Vec2 a, Vec2 b);
Vec2 subs_V2(Vec2 a, Vec2 b);
Vec2 mult_V2(Vec2 a, float scalaire);
float prod_V2(Vec2 a,Vec2 b);
Vec2 divide_V2(Vec2 a, float scalaire);
Vec2 normalize_V2(Vec2 a);
Vec2 mean_V2(const Vec2* vecList, int n);
float norm_V2(Vec2* a);
void print_V2(const Vec2* a);

#endif
