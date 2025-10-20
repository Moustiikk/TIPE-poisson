#include "fish.h"
#include "vector.h"
#include <stdlib.h>
#include <math.h>


Fish init_fish(float x, float y, float speed, int traj_size) {
    Vec2* liste=malloc(traj_size*sizeof(Vec2));
    for(int i=0; i<traj_size; i++){
        liste[i]=init_V2(0.0,0.0);
    }
    File* traj=malloc(sizeof(File));
    traj->values=liste;
    traj->filled=0; 

    float angle = ((float)rand() / RAND_MAX) * 2.0f * (float)M_PI;
    Vec2 v = init_V2(cosf(angle) * speed, sinf(angle) * speed);
    Fish f = {init_V2(x, y), v, traj};
    return f;
}


Simulation init_simulation(int fish_count, int screen_long, int screen_haut, float speed,float body_length,float fov, int traj_size,bool space) {
    Simulation sim;
    sim.fish_count = fish_count;
    sim.screen_long = screen_long;
    sim.screen_haut = screen_haut;
    sim.body_length=body_length;
    sim.speed = speed;
    sim.fov=fov;
    sim.traj_size=traj_size;
    sim.space=space;
    sim.population = malloc(fish_count * sizeof(Fish));
    for (int i = 0; i < fish_count; i++) {
        float x = (float)(rand() % screen_long);
        float y = (float)(rand() % screen_haut);
        sim.population[i] = init_fish(x, y, speed, traj_size);
    }
    return sim;
}

void destroy_simulation(Simulation* sim){
    for (int i = 0; i < sim->fish_count; i++) {
        if (sim->population[i].traj) {
            free(sim->population[i].traj->values);
            free(sim->population[i].traj);
        }
    }
    free(sim->population);
}


int nb_fish_zone(const Fish * population, const Fish *f, int fish_count, float rmin, float rmax,float fov){
    int nb = 0;
    for (int i = 0; i < fish_count; i++){
        if (&population[i]!=f){
            Vec2 rij = subs_V2(population[i].VecPosition, f->VecPosition);
            float d = norm_V2(&rij);
            float theta = angle_V2(f->VecVitesse, rij);
            if ((d <= rmax) && (d > rmin)&& theta<fov) {
                nb++;
            }
        }
    }
    return nb;
}


Fish* neighbours (int neighbour_count,const Fish * population,const Fish *f, int fish_count, float rmin, float rmax,float fov){
    Fish * tab = malloc(neighbour_count * sizeof(Fish));
    int j=0;
    for (int i = 0; i < fish_count; i++){
        if (&population[i]!=f){
            Vec2 rij = subs_V2(population[i].VecPosition, f->VecPosition);
            float d = norm_V2(&rij);
            float theta = angle_V2(f->VecVitesse, rij);
            if ((d <= rmax) && (d > rmin)&& (theta < fov)){
                tab[j] = population[i];
                j++;
            }
        }
    }
    return tab;
}


Vec2 repulsion(const Fish* f, const Fish* population, int fish_count, float r_repulsion,float fov){
    int n_Repulsion = nb_fish_zone(population, f, fish_count, 0.0f, r_repulsion,fov);
    Fish * neighbour_rep = neighbours(n_Repulsion, population, f, fish_count, 0.0f, r_repulsion,fov);
    Vec2 repulsion_vector = init_V2(0.0f, 0.0f);
    for (int i = 0; i < n_Repulsion; i++){
        Vec2 diff = subs_V2(neighbour_rep[i].VecPosition, f->VecPosition);
        repulsion_vector = add_V2(repulsion_vector, normalize_V2(diff));
    }
    free(neighbour_rep);
    return mult_V2(normalize_V2(repulsion_vector), -1.0f);
}




Vec2 alignment(const Fish* f, const Fish* population, int fish_count, float r_repulsion, float r_alignment,float fov){       //alignement est appelé orientation dans la these de gautrais
    int n_Alignment = nb_fish_zone(population, f, fish_count, r_repulsion, r_alignment,fov);
    Fish* neighbours_Alignment = neighbours(n_Alignment, population, f, fish_count, r_repulsion, r_alignment,fov);

    Vec2 alignment_vector = init_V2(0.0f, 0.0f);
    for (int i = 0; i < n_Alignment; i++){
        alignment_vector = add_V2(alignment_vector, neighbours_Alignment[i].VecVitesse);
    }
    free(neighbours_Alignment);
    return normalize_V2(alignment_vector);
}




Vec2 attraction(const Fish* f, const Fish* population, int fish_count,float r_alignment, float r_attraction,float fov){
    int n_Attraction = nb_fish_zone(population, f, fish_count, r_alignment, r_attraction,fov);
    Fish * neighbour_Attraction = neighbours(n_Attraction, population, f, fish_count, r_alignment, r_attraction,fov);
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
                 float r_repulsion, float r_alignment, float r_attraction,float fov){

    Vec2 direction_vector = init_V2(0.0f, 0.0f);
    if (nb_fish_zone(population, f, fish_count, 0.0f, r_repulsion,fov) != 0){
        direction_vector = repulsion(f, population, fish_count, r_repulsion,fov);
    }
    else{
        direction_vector = add_V2(alignment(f, population, fish_count, r_repulsion, r_alignment,fov),
                                  attraction(f, population, fish_count, r_alignment, r_attraction,fov));
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
        sign =1;  
    }
    else if (V.x*D.y - D.x*V.y < 0.0f){
        sign =-1;
    }
    else{ sign=0;}

    float delta_phi_eff = sign*fminf(delta_phi, phi_max);
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


void bounded_repositioning(Vec2* position, Vec2* vitesse, int screen_long, int screen_haut) {

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


void continious_repositioning(Vec2* position, Vec2* vitesse, int screen_long, int screen_haut) {

    // Si le poisson sort par la droite, il réapparaît à gauche
    if (position->x > screen_long) {
        position->x -= screen_long;
    }
    // S’il sort par la gauche, il réapparaît à droite
    else if (position->x < 0.0f) {
        position->x += screen_long;
    }

    // Si le poisson sort par le bas, il réapparaît en haut
    if (position->y > screen_haut) {
        position->y -= screen_haut;
    }
    // S’il sort par le haut, il réapparaît en bas
    else if (position->y < 0.0f) {
        position->y += screen_haut;
    }

    // Aucun changement de direction : vitesse inchangée
}


void update_fish(int i, Simulation* sim, float curvature){
    Fish* f=&sim->population[i];
    Vec2 D=direction_vec(f, sim->population, sim->fish_count, sim->r_repulsion, sim->r_alignment, sim->r_attraction,sim->fov); // nouveau vecteur vitesse 
    Vec2 vitesse_tplus1 = update_vi(D,f->VecVitesse, curvature, sim->speed); // vecteur vitesse t+1 max selon la curvature
    f->VecVitesse = vitesse_tplus1;
    f->VecPosition = add_V2(f->VecPosition, mult_V2(f->VecVitesse, sim->speed));

    if (sim->space){
        continious_repositioning(&f->VecPosition, &f->VecVitesse, sim->screen_long, sim->screen_haut);
    }
    else {
        bounded_repositioning(&f->VecPosition, &f->VecVitesse, sim->screen_long, sim->screen_haut);
    }

    if (f->traj->filled<sim->traj_size){
        f->traj->values[f->traj->filled]=f->VecPosition;
        f->traj->filled++;
    }
    else {
        for (int i=0;i<sim->traj_size-1;i++){
            f->traj->values[i]=f->traj->values[i+1];
        }
        f->traj->values[sim->traj_size-1]=f->VecPosition;
    }
}
