#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>

#include "dependencies/vector.h"
#include "dependencies/fishCON.h"


int main (void){
    int nmb_simulations;
    int duration_simulation;
    printf("Entrez le nombre de simulations à lancer : \n>>> ");
    scanf("%d", &nmb_simulations);
    printf("Entrez la durée de chaque simulations (en secondes): \n>>> ");
    scanf("%d", &duration_simulation);



    int W = 1800, H = 1800;
    float body_length = 8.0*H/900;
    float curvature = 0.10/body_length; // angle max pour lequel le poisson peut tourner
    float r_repulsion = 1.2f  * body_length;
    float r_alignment  = 12.0f * body_length;
    float r_attraction = 20.0f * body_length;
    float fov=90.0*(M_PI/180);
    int traj_size=7;

    bool space=false; //true - > l'espace est continu (périodique) false - > l'espace est férmé rebond

    float velocity= 25.0*body_length*(16.0/1000.0);

    int nb_fish= 200;


   

    for (int nmb_runned=0; nmb_runned<nmb_simulations; nmb_runned++){
        Simulation sim = init_simulation(r_repulsion, r_alignment, r_attraction, nb_fish, W, H, velocity, body_length, fov,traj_size,space);
        for (int t=0; t<duration_simulation*3750; t++){
            Simulation temp_sim =init_simulation(r_repulsion, r_alignment, r_attraction, nb_fish, W, H, velocity, body_length, fov,traj_size,space);
            for (int i = 0; i < temp_sim.fish_count; ++i){
                temp_sim.population[i].VecPosition=sim.population[i].VecPosition;
                temp_sim.population[i].VecVitesse=sim.population[i].VecVitesse;
                temp_sim.population[i].traj=sim.population[i].traj;
            }

            for (int i = 0; i < sim.fish_count; ++i) {
                update_fish(i, &sim, &temp_sim, curvature);
            }

            for (int i = 0; i < temp_sim.fish_count; ++i){
                sim.population[i].VecPosition=temp_sim.population[i].VecPosition;
                sim.population[i].VecVitesse=temp_sim.population[i].VecVitesse;
                sim.population[i].traj=temp_sim.population[i].traj;
            }
            destroy_simulation(&temp_sim);
        }
        destroy_simulation(&sim);
        printf("Simulation %d/%d terminée.\n", nmb_runned+1, nmb_simulations);
        fflush(stdout); 
    }

}

