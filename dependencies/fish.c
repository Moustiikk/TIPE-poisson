#include "fish.h"
#include "vector.h"
#include <stdlib.h>
#include <math.h>


Fish init_fish(float x, float y, float speed) {
    float angle = ((float)rand() / RAND_MAX) * 2.0 * M_PI;
    Vec2 v = {cosf(angle) * speed, sinf(angle) * speed};
    Fish f = {{x, y}, v};
    return f;
}


Simulation init_simulation(int fish_count, int screen_long, int screen_haut, float speed) {
    Simulation sim;
    sim.fish_count = fish_count;
    sim.screen_long = screen_long;
    sim.screen_haut = screen_haut;
    sim.speed = speed;
    sim.population = malloc(fish_count * sizeof(Fish));
    for (int i = 0; i < fish_count; i++) {
        float x = (float)(rand() % screen_long);
        float y = (float)(rand() % screen_haut);
        sim.population[i] = init_fish(x, y, speed);
    }
    return sim;
}


int nb_fish_zone(const Fish * population, const Fish *f, int fish_count, float rmin, float rmax){
    int nb = 0
    for (int i = 0; i<fish_count, i ++){
        if((norm_V2(subs_V2(population[i]->VecPosition,f->VecPosition))<=rmax)&& ((norm_V2(subs_V2(population[i]->VecPosition,f->VecPosition))>rmin))){
            nb++;
        }
    }
    return nb;
}


Fish * neighbours (int neighbour_count,const Fish * population, const Fish *f, int fish_count, float rmin, float rmax){
    Fish * tab = malloc(neighbour_count*sizeof(Fish));
    for (int i = 0 ;i<neighbour_count;i++){
    if((norm_V2(subs_V2(population[i]->VecPosition,f->VecPosition))<=rmax)&& ((norm_V2(subs_V2(population[i]->VecPosition,f->VecPosition))>rmin))){
        tab[i] = population[i];
    }
    }
    return tab;
}


Vec2 repulsion(const Fish* f, const Fish* population, int fish_count, float r_repulsion){
    n_Repulsion = nb_fish_zone(population,f,fish_count,0,r_repulsion);
    Fish * neighbour_rep= neighbours (n_Repulsion,population,f,fish_count,0,r_repulsion);
    Vec2 repulsion_vector = init_V2(0.0,0.0);
    for (int i = 0; i<n_Repulsion; i++){
        repulsion_vector = repulsion_vector + divide_V2(subs_V2(population[i]->VecPosition,f->VecPosition),norm_V2(subs_V2(population[i]->VecPosition,f->VecPosition)));
    }
    return mutl_v2(repulsion_vector,-1.0);
}




Vec2 alignment(const Fish* f, const Fish* population, int fish_count, float r_alignment, float r_repulsion){       //alignement est appelé orientation dans la these de gautrais
    int n_Alignment=nb_fish_zone(population,f,fish_count,r_repulsion,r_alignment);
    Fish* neighbours_Alignment=neighbours(nAlignment,population,f,fish_count,r_repulsion,r_alignment);
    Vec2 alignment_vector=init_V2(0.0,0.0);
    for (int i=0,i<n_Alignment,i++){
        alignment_vector=
    }
};




Vec2 attraction(const Fish* f, const Fish* population, int fish_count, float r_attraction)
{
    
};

