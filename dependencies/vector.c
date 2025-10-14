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
    return a.x*b.x+a.y*b.y;
}

Vec2 divide_V2(Vec2 a, float scalaire) {
    if (scalaire==0){
        return a;
    }
    else{
        float inv_scalaire=1/scalaire;
        return init_V2(a.x * inv_scalaire, a.y * inv_scalaire);
    }
}

Vec2 normalize_V2(Vec2 a){ // vecteur unitaire associé
    return divide_V2(a,norm_V2(&a));
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


float norm_V2(Vec2* a) {
    return sqrtf(a->x * a->x + a->y * a->y);
}


float angle_V2(Vec2 a, Vec2 b){
    float cos_theta=prod_V2(a,b) / norm_V2(&a) * norm_V2(&b);
        if (cos_theta > 1.0f){
            cos_theta=1.0f;
        }
        if (cos_theta < -1.0f){
            cos_theta=-1.0f;
        }

        return acosf(cos_theta);
}



void print_V2(const Vec2* a) {
    printf("Vec2(%.2f, %.2f)\n", a->x, a->y);
}
