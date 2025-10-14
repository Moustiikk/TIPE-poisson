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

    const int W = 900, H = 900;

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
    float body_length = 6.0f;
    float curvature = 0.13/body_length;
    float r_repulsion = 1.0f  * body_length;
    float r_alignment  = 10.0f * body_length;
    float r_attraction = 20.0f * body_length;

    float velocity= 3.0f;

    int nb_fish= 4;


    Simulation sim = init_simulation(nb_fish, W, H, velocity, body_length);
    sim.r_repulsion  = r_repulsion;
    sim.r_alignment  = r_alignment;
    sim.r_attraction = r_attraction;

    bool is_alone = false;
    bool is_together = false;
    bool is_skipping=false;
    bool running = true;

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
                    sim.r_repulsion = 70.0+r_repulsion;
                    sim.r_alignment  = 70.0+r_alignment;
                    sim.r_attraction = 70.0+r_attraction;
                }
                if (evt.key.key == SDLK_T && is_alone) {
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
            }
        }


        for (int i = 0; i < sim.fish_count; ++i) {
            update_fish(&sim.population[i], sim.population, sim.fish_count,
                        sim.r_repulsion, sim.r_alignment, sim.r_attraction,
                        sim.speed, sim.screen_long, sim.screen_haut, curvature);
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
        }

        SDL_RenderPresent(renderer);
        if (is_skipping){
            SDL_Delay(0.0000);
        }
        else{
            SDL_Delay(16);
        }
        
    }

    free(sim.population);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

