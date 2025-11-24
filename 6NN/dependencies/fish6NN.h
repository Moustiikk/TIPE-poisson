#ifndef FISH_H
#define FISH_H

#include "vector.h"

typedef struct{
    Vec2* values;
    int filled;
}File;

typedef struct {
    Vec2 VecPosition;   
    Vec2 VecVitesse;
    File* traj;

} Fish;

typedef struct {
    Fish* population;        
    int fish_count;     
    int screen_long;
    int screen_haut;
    float speed;    // Tjr constante
    float fov;
    float body_length;
    int traj_size;
    bool space;
    float r_visu;
} Simulation;

// Init
Fish init_fish(float x, float y, int traj_size);
Simulation init_simulation(float r_visu, int fish_count, int screen_long, int screen_haut, float speed, float body_length, float fov,int traj_size,bool space);
void destroy_simulation(Simulation* sim);

int nb_fish_zone(const Fish* population, const Fish* fish, int fish_count, float rmin, float rmax,float fov);
Fish* six_neighbours(int neighbour_count, const Fish* population, const Fish* fish,
                        int fish_count, float rmin, float rmax,float fov);



// Update
void update_fish(int i,Simulation* r_sim, Simulation* w_sim, float curvature);

#endif
