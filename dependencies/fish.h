#ifndef FISH_H
#define FISH_H

#include "vector.h"

typedef struct {
    Vec2 VecPosition;   
    Vec2 VecVitesse;  
} Fish;

typedef struct {
    Fish* population;        
    int fish_count;     
    int screen_long;
    int screen_haut;
    float speed;    // Tjr constante
    float blind_zone;
    float body_length;
    float r_alignment;
    float r_attraction;
    float r_repulsion;
} Simulation;

// Init
Fish init_fish(float x, float y, float speed);
Simulation init_simulation(int fish_count, int screen_long, int screen_haut, float speed, float body_length);

int nb_fish_zone(const Fish* population, const Fish* fish, int fish_count, float rmin, float rmax);
Fish* neighbours(int neighbour_count, const Fish* population, const Fish* fish,
                        int fish_count, float rmin, float rmax);

Vec2 repulsion(const Fish* f, const Fish* population, int fish_count, float r_repulsion);
Vec2 alignment(const Fish* f, const Fish* population, int fish_count, float r_alignment, float r_repulsion);
Vec2 attraction(const Fish* f, const Fish* population, int fish_count, float r_alignment, float r_attraction);

// Update
void update_fish(Fish* f, const Fish* population, int fish_count,
                 float r_repulsion, float r_alignment, float r_attraction,
                 float speed, int screen_long, int screen_haut, float k);

#endif
