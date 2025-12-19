#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>
#include <assert.h>
#include <omp.h>
#include "time.h"

#include "dependencies/vector.h"
#include "dependencies/fishCON.h"

typedef struct {
    float polarization;
    float rotation;
    int   population_size;
} SimResult;


float global_polarization(Simulation* sim){
    Vec2 mean_v=init_V2(0.0f,0.0f);
    for(int i=0;i<sim->fish_count;i++){
        mean_v=add_V2(mean_v,normalize_V2(sim->population[i].VecVitesse));
    }
    mean_v=divide_V2(mean_v,sim->fish_count);
    return norm_V2(&mean_v);
}

float global_rotation(Simulation* sim){
    Vec2 barycentre=init_V2(0.0f,0.0f);
    for(int i=0;i<sim->fish_count;i++){
        barycentre=add_V2(barycentre,sim->population[i].VecPosition);
    }
    barycentre=divide_V2(barycentre,sim->fish_count);

    float prod_mean=0.0f;
    for(int i=0;i<sim->fish_count;i++){
        Vec2 ri=subs_V2(sim->population[i].VecPosition,barycentre);
        Vec2 vi=normalize_V2(sim->population[i].VecVitesse);
        ri= normalize_V2(ri);
        prod_mean += (ri.x*vi.y - ri.y*vi.x);
    }
    prod_mean=prod_mean/sim->fish_count;

    if (prod_mean<0.0f){
        prod_mean = -prod_mean;
    }
    return prod_mean;
}


int* get_random_population(int nb_sim, int max_fish, int min_fish){
    int* populations = malloc(nb_sim * sizeof(int));
    assert(populations!=NULL);
    for (int i=0; i<nb_sim; i++){
        populations[i] = min_fish + rand() % (max_fish - min_fish + 1);
    }
    return populations;
}


Vec2** get_random_positons(int nb_sim, int* populations, int screen_long, int screen_haut){
    Vec2** positions = malloc(nb_sim * sizeof(Vec2*));
    assert(positions!=NULL);
    for (int i=0; i<nb_sim; i++){
        positions[i] = malloc(populations[i] * sizeof(Vec2));
        assert(positions[i]!=NULL);
        for (int j=0; j<populations[i]; j++){
            float x = ((float)rand() / RAND_MAX) * screen_long;
            float y = ((float)rand() / RAND_MAX) * screen_haut;
            positions[i][j] = init_V2(x, y);
        }
    }
    return positions;
}


int* put_maxi_first (int* tab, int n){
    int maxi=tab[0];
    int index_maxi=0;
    for (int i=1; i<n; i++){
        if (tab[i]>maxi){
            maxi=tab[i];
            index_maxi=i;
        }
    }
     int tmp=tab[0];
    tab[0]=maxi;
    tab[index_maxi]=tmp;
    return tab;
}




void progress_bar(double fraction) {
    int width = 70; 
    int filled = (int)(fraction * width);
    int percent = (int)(fraction * 100.0);

    printf("\r["); 
    for (int i = 0; i < width; ++i) {
        if (i < filled){
            putchar('#');
        }
        else {
            putchar(' ');
        }
    }
    printf("] %3d%%", percent);
    fflush(stdout);
}



int main (void){
    int nmb_simulations;
    int duration_simulation;
    int num_file;
    int nmb_cores;
    printf("Entrez le nombre de simulations à lancer : \n>>> ");
    scanf("%d", &nmb_simulations);
    printf("Entrez la duree de chaque simulations (en secondes): \n>>> ");
    scanf("%d", &duration_simulation);
    printf("Entrez le nombre de coeurs a utiliser : \n>>> ");
    scanf("%d", &nmb_cores);
    printf("Entrez le numéro du fichier de sauvegarde: \n>>> ");
    scanf("%d", &num_file);

    printf("[DEBUG] nmb_simulations = %d, duree = %d\n", nmb_simulations, duration_simulation);
    
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
    if (f == NULL) {
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


    char filenameResults[50];
    snprintf(filenameResults, sizeof(filenameResults), "calculations/results/result_calculation_%d.csv", num_file);

    FILE* save=fopen(filenameResults, "w");
    if (save == NULL) {
        printf("Erreur d'écriture du fichier de sauvegarde.\n");
        return 1;
    }
    fprintf(save, "simulation,population_size,polarization,rotation\n");
    fclose(save);



    char filenameConfig[50];
    snprintf(filenameConfig, sizeof(filenameConfig), "calculations/configs/config_calculation_%d.txt", num_file);

    FILE* config_save=fopen(filenameConfig, "w");
    if (config_save == NULL) {
        printf("Erreur d'écriture du fichier de configuration.\n");
        return 1;
    }
    fprintf(config_save, "%d", W);
    fprintf(config_save, "%d", H);
    fprintf(config_save, "%d", nb_fish);
    fprintf(config_save, "%f", r_repulsion);
    fprintf(config_save, "%f", r_alignment);
    fprintf(config_save, "%f", r_attraction);
    fprintf(config_save, "%f", curvature);
    fprintf(config_save, "%f", fov);
    fprintf(config_save, "%d", space);
    fclose(config_save);


    float body_length = 8.0*H/900;
    curvature = curvature/body_length;
    r_repulsion = r_repulsion  * body_length;
    r_alignment  = r_alignment * body_length;
    r_attraction = r_attraction * body_length;
    fov=fov*(M_PI/180);
    
    int traj_size=7;

    float velocity= 25.0f*body_length*(16.0f/1000.0f);

    int* populations;
    if (nb_fish==0){
        populations = get_random_population(nmb_simulations, 200, 40);
    }
    else{
        populations = malloc(nmb_simulations * sizeof(int));
        for (int i=0; i<nmb_simulations; i++){
            populations[i] = nb_fish;
        }
    }

    populations = put_maxi_first(populations, nmb_simulations);

    Vec2** positions = get_random_positons(nmb_simulations, populations, W, H);



    omp_set_num_threads(nmb_cores);

    SimResult* results = malloc(nmb_simulations * sizeof(SimResult));
    assert(results!=NULL);

    double* simulations_percentage= malloc(nmb_simulations * sizeof(double));
    assert(simulations_percentage!=NULL);

    for (int i=0; i<nmb_simulations; i++){
        simulations_percentage[i]=0.0;
    }

    time_t start_time = time(NULL);
    printf("[DEBUG] debut simulations\nAvancement : \n");
    //DEBUT BOUCLE PARALLELE
    #pragma omp parallel for schedule(dynamic)
    for (int nmb_runned=0; nmb_runned<nmb_simulations; nmb_runned++){
        int nb_fish_sim= populations[nmb_runned];
        Vec2* positions_fish = positions[nmb_runned];
        

        Simulation sim = init_simulation_w_positions(positions_fish, r_repulsion, r_alignment, r_attraction, nb_fish_sim, W, H, 
            velocity, body_length, fov, traj_size, space);

        
        int pas = duration_simulation * 1000/16;  
        
        for (int t=0; t<pas; t++){
            simulations_percentage[nmb_runned] = (double)(t+1)/pas;
            
            if (omp_get_thread_num() == 0) {
                double total_progress = 0.0;
                for (int i = 0; i < nmb_simulations; i++) {
                    total_progress += simulations_percentage[i];
                }
                total_progress /= nmb_simulations;
                progress_bar(total_progress);
            }

            for (int i = 0; i < sim.fish_count; ++i) {
                update_fish(i, &sim, &sim, curvature);
            }   
        }

        float polarization=global_polarization(&sim);
        float rotation=global_rotation(&sim);

        SimResult result;
        result.polarization = polarization;
        result.rotation = rotation;
        result.population_size = sim.fish_count;
        results[nmb_runned] = result;


        destroy_simulation(&sim);
        fflush(stdout);
    }


    time_t end_time = time(NULL);
    int total_time =end_time - start_time;
    progress_bar(1.0);
    for (int i = 0; i < nmb_simulations; i++) {
        save = fopen(filenameResults, "a");
        if (save == NULL) {
            printf("Erreur d'écriture du fichier de sauvegarde.\n");
            return 1;
        }
        fprintf(save, "%d,%d,%.6f,%.6f\n", i+1, results[i].population_size, results[i].polarization, results[i].rotation);
        fclose(save);
    }
    free(results);
    free(populations);
    for (int i=0; i<nmb_simulations; i++){
        free(positions[i]);
    }
    free(positions);
    

    printf("\rToutes les simulations sont terminees en : %ds                                      \n",total_time);
    printf("Appuyez sur Entree pour quitter...\n");



    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
    getchar();
    return 0;
}
