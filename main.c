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

    const int W = 1920, H = 1080;

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
    float body_length = 15.0f;
    Simulation sim = init_simulation(200, W, H, 3.0f, body_length);
    sim.r_repulsion  = 5.0f  * body_length;
    sim.r_alignment  = 20.0f * body_length;
    sim.r_attraction = 40.0f * body_length;

    bool running = true;
    uint32_t last_ticks = SDL_GetTicks();

    while (running) {
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_EVENT_QUIT) running = false;
        }

        uint32_t now = SDL_GetTicks();
        float delta_time = (now - last_ticks) / 1000.0f;
        last_ticks = now;

        for (int i = 0; i < sim.fish_count; ++i) {
            update_fish(&sim.population[i], sim.population, sim.fish_count,
                        sim.r_repulsion, sim.r_alignment, sim.r_attraction,
                        sim.speed, sim.screen_long, sim.screen_haut, 0.23 * body_length);
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
        SDL_Delay(16);
    }

    free(sim.population);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

