// main.c — SDL3 + simulation de poissons (dependencies/*)
// compile avec le SDK officiel SDL3 MinGW: -I<SDL_PATH>\include -L<SDL_PATH>\lib -lSDL3

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL3/SDL.h>

#include "dependencies/vector.h"
#include "dependencies/fish.h"

int main(void) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    const int W = 950, H = 950;

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

    /* ---- Simulation ---- */
    float body_length = 8.0*H/900;
    float curvature = 0.10/body_length; // angle max pour lequel le poisson peut tourner
    float r_repulsion = 1.2f  * body_length;
    float r_alignment  = 10.0f * body_length;
    float r_attraction = 20.0f * body_length;
    float fov=90.0*(M_PI/180);
    int traj_size=7;

    bool space=false; //true - > l'espace est continu (périodique) false - > l'espace est férmé rebond

    float velocity= 25.0*body_length*(16.0/1000.0);

    int nb_fish= 200;


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

