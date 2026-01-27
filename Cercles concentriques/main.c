// main.c — SDL3 + simulation de poissons (dependencies/*)
// compile avec le SDK officiel SDL3 MinGW: -I<SDL_PATH>\include -L<SDL_PATH>\lib -lSDL3

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>

#include "dependencies/vector.h"
#include "dependencies/fishCON.h"


ConnexityComp* get_main_comp(Simulation* sim, ConnexityResults* connexity_results){
    int max_size=0;
    int index_main_comp=0;
    for (int i=0; i<connexity_results->connexity_count; i++){
        if (connexity_results->connexity_comp[i]->comp_size>max_size){
            max_size=connexity_results->connexity_comp[i]->comp_size;
            index_main_comp=i;
        }
    }
    return connexity_results->connexity_comp[index_main_comp];
}

float local_polarization(Simulation* sim, ConnexityComp* comp){
    Vec2 mean_v=init_V2(0.0f,0.0f);
    for(int i=0;i<comp->comp_size;i++){
        mean_v=add_V2(mean_v,normalize_V2(comp->comp[i].VecVitesse));
    }
    mean_v=divide_V2(mean_v,comp->comp_size);
    return norm_V2(&mean_v);
}


float local_rotation(Simulation* sim, ConnexityComp* comp){
    Vec2 barycentre=init_V2(0.0f,0.0f);
    for(int i=0;i<comp->comp_size;i++){
        barycentre=add_V2(barycentre,comp->comp[i].VecPosition);
    }
    barycentre=divide_V2(barycentre,comp->comp_size);
    float prod_mean=0.0f;
    for(int i=0;i<comp->comp_size;i++){
        Vec2 ri=subs_V2(comp->comp[i].VecPosition,barycentre);
        Vec2 vi=normalize_V2(comp->comp[i].VecVitesse);
        ri= normalize_V2(ri);
        prod_mean += (ri.x*vi.y - ri.y*vi.x);
    }
    prod_mean=prod_mean/comp->comp_size;
    if (prod_mean<0.0f){
        prod_mean = -prod_mean;
    }
    return prod_mean;
}

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


int main(void) {
    int W;
    int H;
    float curvature;
    float r_repulsion;
    float r_alignment;
    float r_attraction;
    float fov;
    bool space;
    int nb_fish;
    int tmpspace;

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
    fscanf(f, "%d", &tmpspace);
    fclose(f);

    space = (tmpspace != 0);


    float body_length = 8.0*H/900;
    curvature = curvature/body_length;
    r_repulsion = r_repulsion  * body_length;
    r_alignment  = r_alignment * body_length;
    r_attraction = r_attraction * body_length;
    fov=fov*(M_PI/180);

    int traj_size=7;

    float velocity= 25.0f*body_length*(16.0f/1000.0f);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }


    SDL_Window* window = SDL_CreateWindow("Banc de poissons (SDL3)", W, H, 0);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }


    Simulation sim = init_simulation(r_repulsion, r_alignment, r_attraction, nb_fish, W, H, velocity, body_length, fov,traj_size,space);

    bool is_alone = false;
    bool is_together = false;
    bool is_skipping=false;
    bool running = true;
    bool is_paused=false;
    bool draw_interactions=false;

    while (running) {
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_EVENT_QUIT){
                running = false;
            }
            else if (evt.type== SDL_EVENT_KEY_DOWN){

                if (evt.key.key == SDLK_Q) {
                    running = false;
                }
                if (evt.key.key == SDLK_S && is_skipping) {
                    is_skipping = false;
                }
                else if (evt.key.key == SDLK_S) {
                    is_skipping = true;
                }
                if (evt.key.key == SDLK_A && is_alone) {
                    is_alone = false;
                    sim.r_repulsion = r_repulsion;
                    sim.r_alignment  = r_alignment;
                    sim.r_attraction = r_attraction;
                    draw_interactions=true;
                }
                else if (evt.key.key == SDLK_A) {
                    is_alone = true;
                    sim.r_repulsion = 700.0+r_repulsion;
                    sim.r_alignment  = 700.0+r_alignment;
                    sim.r_attraction = 700.0+r_attraction;
                    draw_interactions=false;
                }
                if (evt.key.key == SDLK_T && is_together) {
                    is_together = false;
                    sim.r_repulsion = r_repulsion;
                    sim.r_alignment  = r_alignment;
                    sim.r_attraction = r_attraction;
                }
                else if (evt.key.key == SDLK_T) {
                    is_together = true;
                    sim.r_repulsion = r_repulsion;
                    sim.r_alignment  = r_alignment;
                    sim.r_attraction = 1000.0+r_attraction;
                }
                if (evt.key.key == SDLK_P) {
                    is_paused=true;
                    while (is_paused) {
                        SDL_Event evt;
                        if (SDL_WaitEventTimeout(&evt, -1)) {
                            if (evt.type == SDL_EVENT_QUIT) {
                                running = false;
                                is_paused = false;
                            } 
                            else if (evt.type == SDL_EVENT_KEY_DOWN && evt.key.key == SDLK_P) {
                                is_paused = false; 
                            }
                        }
                    }
                }
                if (evt.key.key == SDLK_G && draw_interactions) {
                    draw_interactions = false;
                }
                else if (evt.key.key == SDLK_G) {
                    draw_interactions = true;
                }
            }
                
        }
        

        for (int i = 0; i < sim.fish_count; ++i) {
            update_fish(i, &sim, &sim, curvature);
        }
        

        SDL_SetRenderDrawColor(renderer, 10, 12, 30, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 210, 60, 255);
        for (int i = 0; i < sim.fish_count; ++i) {
            SDL_FRect fish_rect = {
                sim.population[i].VecPosition.x - body_length / 2.0f,
                sim.population[i].VecPosition.y - body_length / 2.0f,
                body_length,
                body_length
            };
            SDL_RenderFillRect(renderer, &fish_rect);
            for (int j=0;j<sim.population[i].traj->filled;j++){
                SDL_FRect pos_traj ={
                    sim.population[i].traj->values[j].x - body_length/(4.0f*2.0f),
                    sim.population[i].traj->values[j].y - body_length/(4.0f*2.0f),
                    body_length/4.0f,
                    body_length/4.0f
                };
                SDL_RenderFillRect(renderer, &pos_traj);
            }
        }
        

        ConnexityResults connexity_results = get_connexity_comp(&sim);
        ConnexityComp* main_comp = get_main_comp(&sim, &connexity_results);
        float polarization;
        float rotation ;

        if (main_comp->comp_size>= 50) {
            polarization = local_polarization(&sim, main_comp);
            rotation = local_rotation(&sim, main_comp);
        } 
        else {
            polarization = global_polarization(&sim);
            rotation = global_rotation(&sim);
        }


        bool is_swarming= false;
        bool is_schooling= false;
        bool is_milling= false;
        if (polarization>0.75f){
            is_schooling=true;
        }
        else if (rotation>0.6f && polarization<0.5f){
            is_milling=true;
        }
        else if (rotation<=0.6f && polarization<0.5f){
            is_swarming=true;
        }
        if (is_schooling){
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 80); // Vert
        }
        else if (is_milling){
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 80); // Bleu
        }
        else if (is_swarming){
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 80); // Jaune
        }
        else{
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 80); // Rouge
        }

        SDL_FRect status_rect = {10.0f, 10.0f, 25.0f, 25.0f};
        SDL_RenderFillRect(renderer, &status_rect);

        if (draw_interactions) {
            SDL_SetRenderDrawColor(renderer, 100, 100, 255, 50);
            Graph* g = get_interaction_graph(&sim);

            for (int i = 0; i < sim.fish_count; i++) {
                for (int j = 0; j < sim.fish_count; j++) {
                    if (g->g[i][j] == 1) {
                        SDL_RenderLine(renderer,
                            sim.population[i].VecPosition.x,
                            sim.population[i].VecPosition.y,
                            sim.population[j].VecPosition.x,
                            sim.population[j].VecPosition.y
                        );
                    }
                }
            }

            destroy_graph(g);
        }


        
        SDL_RenderPresent(renderer);

        if (is_skipping){
            SDL_Delay(0.0000);
        }
        else{
            SDL_Delay(16);
        }
       
        
    }

    destroy_simulation(&sim);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

