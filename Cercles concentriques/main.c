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
                }
                else if (evt.key.key == SDLK_A) {
                    is_alone = true;
                    sim.r_repulsion = 700.0+r_repulsion;
                    sim.r_alignment  = 700.0+r_alignment;
                    sim.r_attraction = 700.0+r_attraction;
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

        float polarization=global_polarization(&sim);
        float rotation=global_rotation(&sim);
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


        SDL_FRect status_rect = {10.0f, 10.0f, 50.0f, 50.0f};
        SDL_RenderFillRect(renderer, &status_rect);




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

