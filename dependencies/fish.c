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
        if((norm_V2(&subs_V2(population[i]->VecPosition,f->VecPosition))<=rmax)&& ((norm_V2(&subs_V2(population[i]->VecPosition,f->VecPosition))>rmin))){
            nb++;
        }
    }
    return nb;
}


Fish * neighbours (int neighbour_count,const Fish * population, const Fish *f, int fish_count, float rmin, float rmax){
    Fish * tab = malloc(neighbour_count*sizeof(Fish));
    for (int i = 0 ;i<neighbour_count;i++){
    if((norm_V2(&subs_V2(population[i]->VecPosition,f->VecPosition))<=rmax)&& ((norm_V2(&subs_V2(population[i]->VecPosition,f->VecPosition))>rmin))){
        tab[i] = population[i];
    }
    }
    return tab;
}


Vec2 repulsion(const Fish* f, const Fish* population, int fish_count, float r_repulsion){
    n_Repulsion = nb_fish_zone(population,f,fish_count,0.0,r_repulsion);
    Fish * neighbour_rep= neighbours (n_Repulsion,population,f,fish_count,0,r_repulsion);
    Vec2 repulsion_vector = init_V2(0.0,0.0);
    for (int i = 0; i<n_Repulsion; i++){
        repulsion_vector = add_V2(repulsion_vector, divide_V2(subs_V2(neighbour_rep[i]->VecPosition,f->VecPosition),norm_V2(&subs_V2(neighbour_rep[i]->VecPosition,f->VecPosition))));
    }
    return mutl_v2(repulsion_vector,-1.0);
}




Vec2 alignment(const Fish* f, const Fish* population, int fish_count, float r_alignment, float r_repulsion){       //alignement est appelé orientation dans la these de gautrais
    int n_Alignment=nb_fish_zone(population,f,fish_count,r_repulsion,r_alignment);
    Fish* neighbours_Alignment=neighbours(n_Alignment,population,f,fish_count,r_repulsion,r_alignment);

    Vec2 alignment_vector=init_V2(0.0,0.0);
    for (int i=0,i<n_Alignment,i++){
        alignment_vector=add_V2(alignement_vector, neighbours_Alignment[i]->VecVitesse);
    }
    return alignement_vector
}




Vec2 attraction(const Fish* f, const Fish* population, int fish_count,float r_alignment, float r_attraction){
    n_Attraction = nb_fish_zone(population,f,fish_count,r_alignment,r_attraction);
    Fish * neighbour_Attraction= neighbours(n_Attraction,population,f,fish_count,r_alignment,r_attraction);
    Vec2 attraction_vector = init_V2(0.0,0.0);
    for (int i = 0; i<n_Attraction; i++){
        attraction_vector = add_V2(attraction_vector, divide_V2(subs_V2(neighbour_Attraction[i]->VecPosition,f->VecPosition),norm_V2(&subs_V2(neighbour_Attraction[i]->VecPosition,f->VecPosition))));
    }
    return attraction_vector;
}

Vec2 direction_vec (Fish* f, const Fish* population, int fish_count,
                 float r_repulsion, float r_alignment, float r_attraction){
    Vec2 direction_vector = init_V2(0.0,0.0);
        if(nb_fish_zone(population,f,fish_count,0.0,r_repulsion) != 0){
            direction_vector = repulsion(f,population,fish_count,r_repulsion);
        }
        else{
            direction_vector = add_V2(alignment(f,population,fish_count,r_repulsion,r_alignment), attraction(f,population,fish_count,r_alignment,r_attraction));
        }
    return direction_vector;

}

float turning_angle (Vec2 D,Vec2 V,float k, float speed){
    float phi_max = k*speed;
    float delta_phi = acos((prod_V2(D,V))/norm_V2(D)*norm_V2(V));
    float sign;
    if (V.x*D.y-D.x*V.y>0.0){
        sign=1;
    }
    else if(V.x*D.y-D.x*V.y<0.0){
        sign=-1;
    }
    else {
        return 0.0;
    }
    float delta_phi_eff = sign*min(delta_phi, phi_max);
    return delta_phi_eff;
}

Vec2 update_vi (Vec2 D,Vec2 V,float k, float speed){
    float angle = turning_angle(Vec2 D,Vec2 V,float k, float speed);
    Vec2 vi_tplus1= {V.x*cos(angle)-V.y*sin(angle), V.x*sin(angle) + V.y*cos(angle)}; // calcul matriciel Vi_tplus1 = matrice rotation * Vi
    vi_tplus1=mult_V2(vi_tplus1,(1/norm_V2(vi_tplus1)));
    return vi_tplus1;

}


Vec2 repositioning(Vec2 fish_position, int screen_long, int screen_haut){
    Vec2 position=fish_position;
    if (position.x < 0){
        position.x = (float)screen_long;
    }
    
    if (position.x > screen_long){
        position.x = 0.0f;
    }

    
    if (position.y < 0){
        position.y = (float)screen_haut;
    }

    if (position.y > screen_haut){
        position.y = 0.0f;
    }

    return position;
}


void update_fish(Fish* f, const Fish* population, int fish_count,
                 float r_repulsion, float r_alignment, float r_attraction,
                 float speed, int screen_long, int screen_haut,float k);{
    Vec2 vitesse_tplus1 =update_vi(direction_vector(f,population,fish_count,r_repulsion,r_alignment,r_attraction),f->VecVitesse,k,speed)
    f->VecVitesse=vitesse_tplus1
    Vec2 position_tplus1 = add_V2(f->VecPosition, mult(f->VecVitesse,speed)) ;
    f->VecPosition=repositioning(position_tplus1, screen_long, screen_haut);
}
