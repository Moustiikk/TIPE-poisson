#include "fish6NN.h"
#include "vector.h"
#include <stdlib.h>
#include <math.h>


Fish init_fish(float x, float y, int traj_size) {
    Vec2* liste=malloc(traj_size*sizeof(Vec2));
    for(int i=0; i<traj_size; i++){
        liste[i]=init_V2(0.0,0.0);
    }
    File* traj=malloc(sizeof(File));
    traj->values=liste;
    traj->filled=0; 

    float angle = ((float)rand() / RAND_MAX) * 2.0f * (float)M_PI;
    Vec2 v = init_V2(cosf(angle), sinf(angle));
    Fish f = {init_V2(x, y), v, traj};
    return f;
}


Simulation init_simulation(float r_visu, int fish_count, int screen_long, int screen_haut, float speed, float body_length, float fov,int traj_size,bool space) {
    Simulation sim;
    sim.r_visu=r_visu;
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
        sim.population[i] = init_fish(x, y, traj_size);
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
        if (&(population[i])!=f){
            Vec2 rij = subs_V2(population[i].VecPosition, f->VecPosition);
            float d = norm_V2(&rij);
            float theta = angle_V2(f->VecVitesse, normalize_V2(rij));
            if ((d <= rmax) && (d > rmin)&& theta<fov) {
                nb++;
            }
        }
    }
    return nb;
}
    


void add_fish (Fish* six_fish_list,Fish* f, int index_fish, Fish* population, int fish_count){
    Vec2 r0j = subs_V2(six_fish_list[0].VecPosition, f->VecPosition);
    float max_distance = norm_V2(&r0j);
    int i_max=0;
    
    for (int i=1; i<6; i++){
        Vec2 rij = subs_V2(six_fish_list[i].VecPosition, f->VecPosition);
        if (norm_V2(&rij)>max_distance){
            max_distance=norm_V2(&rij);
            i_max=i;
        }
    }
    
    Vec2 v= subs_V2(population[index_fish].VecPosition, f->VecPosition);
    if (norm_V2(&v)<max_distance){
        six_fish_list[i_max]=population[index_fish];
    }
}

Fish* six_neighbours (int neighbour_count,const Fish * population,const Fish *f, int fish_count, float rmin, float rmax,float fov){ 
    // ajoute ou non le rayon du poisson en argument si il est inférieur au max des rayons
    Fish* tab;                                    

    if (neighbour_count<=6){
        tab = malloc(neighbour_count * sizeof(Fish));
        int j=0;
        for (int i = 0; i < fish_count; i++){
            if (&population[i]!=f){
                Vec2 rij = subs_V2(population[i].VecPosition, f->VecPosition);
                float d = norm_V2(&rij);
                float theta = angle_V2(f->VecVitesse, normalize_V2(rij));
                if ((d <= rmax) && (d > rmin)&& (theta < fov)){
                    tab[j] = population[i];
                    j++;
                }
            }
        }
    }
    else {
        tab = malloc(6 * sizeof(Fish));
        int filled=0;
        for (int i = 0; i < fish_count; i++){
            if (&population[i]!=f){
                Vec2 rij = subs_V2(population[i].VecPosition, f->VecPosition);
                float d = norm_V2(&rij);
                float theta = angle_V2(f->VecVitesse, normalize_V2(rij));
                if ((d <= rmax) && (d > rmin)&& (theta < fov)){
                    if (filled<=5){
                        tab[filled]=population[i];
                        filled++;
                    }
                    else{
                        add_fish(tab, f, i, population, fish_count);
                    }
                }
            }
        }
    }

    
    return tab;
}



Vec2 direction_vec (Fish* f, const Fish* population, int fish_count,
                 float r_visu,float fov){

    Vec2 direction_vector = init_V2(0.0f, 0.0f);
    
    int neighbour_count=nb_fish_zone(population,f,fish_count,0.0, r_visu,fov);
    int nb_nearest_nei = neighbour_count;

    if (nb_nearest_nei>6){
        nb_nearest_nei=6;
    }

    if (nb_nearest_nei == 0){
        return f->VecVitesse;
    }
    else{
        Vec2* six_VecVitesse = malloc(nb_nearest_nei*sizeof(Vec2));
        Fish* six_fish_list=six_neighbours(neighbour_count, population,f, fish_count, 0.0,r_visu ,fov);

        for (int i=0; i<nb_nearest_nei;i++){
            six_VecVitesse[i]=six_fish_list[i].VecVitesse;
        }
        direction_vector = normalize_V2(mean_V2(six_VecVitesse,nb_nearest_nei));
        free(six_VecVitesse);
        free(six_fish_list);
        
        return normalize_V2(direction_vector);
    }
    

}

float turning_angle (Vec2 D,Vec2 V,float curvature, float speed){
    float phi_max = curvature * speed;
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

Vec2 update_vi (Vec2 D,Vec2 V,float curvature, float speed){
    float angle = turning_angle(D, V, curvature, speed);
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

static float soft_gain(float d, float dmax) {
    if (d >= dmax) return 0.0f;
    float x = 1.0f - d / dmax;   // 0 à dmax -> 1 à 0
    return x * x;                // gain croît quand on s’approche du mur
}
static void wall_soft_repulsion(Vec2* position, Vec2* vitesse,
                                int W, int H,
                                float dmax, float k)
{
    float dxL = position->x;              // distance au mur gauche
    float dxR = (float)W - position->x;   // droite
    float dyB = position->y;              // bas
    float dyT = (float)H - position->y;   // haut

    Vec2 steer = (Vec2){0.0f, 0.0f};

    if (dxL < dmax) {
        steer = add_V2(steer, mult_V2((Vec2){+1.0f, 0.0f}, k * soft_gain(dxL, dmax)));
    }
    if (dxR < dmax) {
        steer = add_V2(steer, mult_V2((Vec2){-1.0f, 0.0f}, k * soft_gain(dxR, dmax)));
    }
    if (dyB < dmax) {
        steer = add_V2(steer, mult_V2((Vec2){0.0f, +1.0f}, k * soft_gain(dyB, dmax)));
    }
    if (dyT < dmax) {
        steer = add_V2(steer, mult_V2((Vec2){0.0f, -1.0f}, k * soft_gain(dyT, dmax)));
    }

    if (steer.x != 0.0f || steer.y != 0.0f) {
        Vec2 v_new = add_V2(*vitesse, steer);
        *vitesse = normalize_V2(v_new);
    }
}


void bounded_repositioning(Vec2* position, Vec2* vitesse, int screen_long, int screen_haut) {

     wall_soft_repulsion(position, vitesse, screen_long, screen_haut,
                        /* dmax */ 30.0f, /* k */ 0.5f);
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


void update_fish(int i, Simulation* r_sim, Simulation* w_sim, float curvature){
    Fish* f=&w_sim->population[i];
    Vec2 D=direction_vec(f, r_sim->population, r_sim->fish_count, r_sim->r_visu,r_sim->fov); // nouveau vecteur vitesse 
    Vec2 vitesse_tplus1 = update_vi(D,f->VecVitesse, curvature, r_sim->speed); // vecteur vitesse t+1 max selon la curvature
    f->VecVitesse = vitesse_tplus1;
    f->VecPosition = add_V2(f->VecPosition, mult_V2(f->VecVitesse, r_sim->speed));

    if (r_sim->space){
        continious_repositioning(&f->VecPosition, &f->VecVitesse, r_sim->screen_long, r_sim->screen_haut);
    }
    else {
        bounded_repositioning(&f->VecPosition, &f->VecVitesse, r_sim->screen_long, r_sim->screen_haut);
    }

    if (f->traj->filled<r_sim->traj_size){
        f->traj->values[f->traj->filled]=f->VecPosition;
        f->traj->filled++;
    }
    else {
        for (int i=0;i<r_sim->traj_size-1;i++){
            f->traj->values[i]=f->traj->values[i+1];
        }
        f->traj->values[r_sim->traj_size-1]=f->VecPosition;
    }
}
