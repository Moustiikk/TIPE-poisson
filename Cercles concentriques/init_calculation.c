#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>
#include "time.h"

#include "dependencies/vector.h"
#include "dependencies/fishCON.h"

void progress_bar(double fraction) {
    int width = 50; 
    int filled = (int)(fraction * width);
    int percent = (int)(fraction * 100.0);

    printf("\r["); 
    for (int i = 0; i < width; ++i) {
        if (i < filled) putchar('#');
        else putchar(' ');
    }
    printf("] %3d%%", percent);
    fflush(stdout);
}

int main (void){
    int nmb_simulations;
    int duration_simulation;
    printf("Entrez le nombre de simulations à lancer : \n>>> ");
    scanf("%d", &nmb_simulations);
    printf("Entrez la duree de chaque simulations (en secondes): \n>>> ");
    scanf("%d", &duration_simulation);

    printf("[DEBUG] nmb_simulations = %d, duree = %d\n", nmb_simulations, duration_simulation);
    fflush(stdout);
    
    int W;
    int H;
    float curvature;
    float r_repulsion;
    float r_alignment;
    float r_attraction;
    float fov;
    bool space;
    int nb_fish;

    FILE *f = fopen("config.txt", "r");
    if (!f) {
        printf("Erreur ouverture fichier.\n");
        return 1;
    }
    fscanf(f, "%d", &W);
    fscanf(f, "%d", &H);
    fscanf(f, "%d", &nb_fish);
    fscanf(f, "%f", &r_repulsion);
    fscanf(f, "%f", &r_alignment);
    fscanf(f, "%f", &r_attraction);
    fscanf(f, "%f", &curvature);
    fscanf(f, "%f", &fov);
    fscanf(f, "%d", &space);
    fclose(f);


    float body_length = 8.0*H/900;
    curvature = curvature/body_length;
    r_repulsion = r_repulsion  * body_length;
    r_alignment  = r_alignment * body_length;
    r_attraction = r_attraction * body_length;
    fov=fov*(M_PI/180);
    
    int traj_size=7;

    float velocity= 25.0f*body_length*(16.0f/1000.0f);

    time_t start_time = time(NULL);

    for (int nmb_runned=0; nmb_runned<nmb_simulations; nmb_runned++){
        printf("[DEBUG] debut simulation %d\n", nmb_runned+1);
        fflush(stdout);

        Simulation sim = init_simulation(r_repulsion, r_alignment, r_attraction,
                                         nb_fish, W, H, velocity, body_length,
                                         fov, traj_size, space);

        printf("[DEBUG] init_simulation OK\n");
        fflush(stdout);

        // pour debug, on réduit le nombre d’itérations
        int pas = duration_simulation * 1000/16;  // au lieu de *3750

        for (int t=0; t<pas; t++){
            progress_bar((double)(t+1)/pas);
            
            for (int i = 0; i < sim.fish_count; ++i) {
                update_fish(i, &sim, &sim, curvature);
            }
        }


        destroy_simulation(&sim);
        printf("\rSimulation %d/%d terminee.                           \n", nmb_runned+1, nmb_simulations);
        fflush(stdout);
    }
    time_t end_time = time(NULL);
    int total_time =end_time - start_time;
    printf("\rToutes les simulations sont terminees en : %ds                           \n",total_time);
    printf("Appuyez sur Entree pour quitter...\n");
    fflush(stdout);

    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
    getchar();
    return 0;
}
