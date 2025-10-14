#include "fish.h"
#include "vector.h"
#include <stdlib.h>
#include <math.h>


Fish init_fish(float x, float y, float speed) {
    float angle = ((float)rand() / RAND_MAX) * 2.0f * (float)M_PI;
    Vec2 v = (Vec2){cosf(angle) * speed, sinf(angle) * speed};
    Fish f = {(Vec2){x, y}, v};
    return f;
}


Simulation init_simulation(int fish_count, int screen_long, int screen_haut, float speed,float body_length,float blind_zone) {
    Simulation sim;
    sim.fish_count = fish_count;
    sim.screen_long = screen_long;
    sim.screen_haut = screen_haut;
    sim.body_length=body_length;
    sim.speed = speed;
    sim.blind_zone=blind_zone;
    sim.population = malloc(fish_count * sizeof(Fish));
    for (int i = 0; i < fish_count; i++) {
        float x = (float)(rand() % screen_long);
        float y = (float)(rand() % screen_haut);
        sim.population[i] = init_fish(x, y, speed);
    }
    return sim;
}


int nb_fish_zone(const Fish * population, const Fish *f, int fish_count, float rmin, float rmax,float blind_zone){
    int nb = 0;
    for (int i = 0; i < fish_count; i++){
        Vec2 rij = subs_V2(population[i].VecPosition, f->VecPosition);
        float d = norm_V2(&rij);
        float theta = angle_V2(f->VecVitesse, rij);
        if ((d <= rmax) && (d > rmin)&& theta<blind_zone) {
            nb++;
        }
    }
    return nb;
}


Fish* neighbours (int neighbour_count,const Fish * population,const Fish *f, int fish_count, float rmin, float rmax,float blind_zone){
    Fish * tab = malloc(neighbour_count * sizeof(Fish));
    int j=0;
    for (int i = 0; i < fish_count; i++){
        Vec2 rij = subs_V2(population[i].VecPosition, f->VecPosition);
        float d = norm_V2(&rij);
        float theta = angle_V2(f->VecVitesse, rij);
        if ((d <= rmax) && (d > rmin)&& (theta < blind_zone)){
            tab[j] = population[i];
            j++;
        }
    }
    return tab;
}


Vec2 repulsion(const Fish* f, const Fish* population, int fish_count, float r_repulsion,float blind_zone){
    int n_Repulsion = nb_fish_zone(population, f, fish_count, 0.0f, r_repulsion,blind_zone);
    Fish * neighbour_rep = neighbours(n_Repulsion, population, f, fish_count, 0.0f, r_repulsion,blind_zone);
    Vec2 repulsion_vector = init_V2(0.0f, 0.0f);
    for (int i = 0; i < n_Repulsion; i++){
        Vec2 diff = subs_V2(neighbour_rep[i].VecPosition, f->VecPosition);
        repulsion_vector = add_V2(repulsion_vector, normalize_V2(diff));
    }
    free(neighbour_rep);
    return mult_V2(normalize_V2(repulsion_vector), -1.0f);
}




Vec2 alignment(const Fish* f, const Fish* population, int fish_count, float r_repulsion, float r_alignment,float blind_zone){       //alignement est appelé orientation dans la these de gautrais
    int n_Alignment = nb_fish_zone(population, f, fish_count, r_repulsion, r_alignment,blind_zone);
    Fish* neighbours_Alignment = neighbours(n_Alignment, population, f, fish_count, r_repulsion, r_alignment,blind_zone);

    Vec2 alignment_vector = init_V2(0.0f, 0.0f);
    for (int i = 0; i < n_Alignment; i++){
        alignment_vector = add_V2(alignment_vector, neighbours_Alignment[i].VecVitesse);
    }
    free(neighbours_Alignment);
    return normalize_V2(alignment_vector);
}




Vec2 attraction(const Fish* f, const Fish* population, int fish_count,float r_alignment, float r_attraction,float blind_zone){
    int n_Attraction = nb_fish_zone(population, f, fish_count, r_alignment, r_attraction,blind_zone);
    Fish * neighbour_Attraction = neighbours(n_Attraction, population, f, fish_count, r_alignment, r_attraction,blind_zone);
    Vec2 attraction_vector = init_V2(0.0f, 0.0f);

    for (int i = 0; i < n_Attraction; i++){
        Vec2 diff = subs_V2(neighbour_Attraction[i].VecPosition, f->VecPosition);
        float d = norm_V2(&diff);
        attraction_vector = add_V2(attraction_vector, divide_V2(diff, d));
    }
    free(neighbour_Attraction);
    return attraction_vector;
}

Vec2 direction_vec (Fish* f, const Fish* population, int fish_count,
                 float r_repulsion, float r_alignment, float r_attraction,float blind_zone){

    Vec2 direction_vector = init_V2(0.0f, 0.0f);
    if (nb_fish_zone(population, f, fish_count, 0.0f, r_repulsion,blind_zone) != 0){
        direction_vector = repulsion(f, population, fish_count, r_repulsion,blind_zone);
    }
    else{
        direction_vector = add_V2(alignment(f, population, fish_count, r_repulsion, r_alignment,blind_zone),
                                  attraction(f, population, fish_count, r_alignment, r_attraction,blind_zone));
    }
    return normalize_V2(direction_vector);

}

float turning_angle (Vec2 D,Vec2 V,float k, float speed){
    float phi_max = k * speed;
    if (norm_V2(&D)==0.0 || norm_V2(&V)==0.0){
        return 0.0f;
    }
    
    

    float delta_phi = angle_V2(D,V);
    float sign;
    if (V.x*D.y - D.x*V.y > 0.0f){
        sign = 1.0f;
    }
    else if (V.x*D.y - D.x*V.y < 0.0f){
        sign = -1.0f;
    }
    else {
        return 0.0f;
    }
    float delta_phi_eff = sign * fminf(delta_phi, phi_max);
    return delta_phi_eff;
}

Vec2 update_vi (Vec2 D,Vec2 V,float k, float speed){
    float angle = turning_angle(D, V, k, speed);
    Vec2 vi_tplus1 = (Vec2){ V.x * cosf(angle) - V.y * sinf(angle),
                             V.x * sinf(angle) + V.y * cosf(angle) }; // calcul matriciel Vi_tplus1 = matrice rotation * Vi
    return normalize_V2(vi_tplus1);

}


Vec2 reflect(Vec2 v, Vec2 normale) {
    float d = prod_V2(v, normale);
    Vec2 r;
    r.x = v.x - 2.0f * d * normale.x;
    r.y = v.y - 2.0f * d * normale.y;
    return r;
}


void repositioning(Vec2* position, Vec2* vitesse, int screen_long, int screen_haut) {

    if (position->x < 0.0f) {
        position->x = 2.0f;
        *vitesse = reflect(*vitesse, (Vec2){1.0f, 0.0f}); 
    }

    else if (position->x > screen_long) {
        position->x = (float)screen_long-2.0;
        *vitesse = reflect(*vitesse, (Vec2){-1.0f, 0.0f}); 
    }

  
    if (position->y < 0.0f) {
        position->y = 2.0f;
        *vitesse = reflect(*vitesse, (Vec2){0.0f, 1.0f}); 
    }
    
    else if (position->y > screen_haut) {
        position->y = (float)screen_haut-2;
        *vitesse = reflect(*vitesse, (Vec2){0.0f, -1.0f}); 
    }
}


void update_fish(int i, Simulation* sim, float curvature){
    Fish* f=&sim->population[i];
    Vec2 D=direction_vec(f, sim->population, sim->fish_count, sim->r_repulsion, sim->r_alignment, sim->r_attraction,sim->blind_zone); // nouveau vecteur vitesse 
    Vec2 vitesse_tplus1 = update_vi(D,f->VecVitesse, curvature, sim->speed); // vecteur vitesse t+1 max selon la curvature
    f->VecVitesse = vitesse_tplus1;
    f->VecPosition = add_V2(f->VecPosition, mult_V2(f->VecVitesse, sim->speed));
    repositioning(&f->VecPosition, &f->VecVitesse, sim->screen_long, sim->screen_haut);
}
