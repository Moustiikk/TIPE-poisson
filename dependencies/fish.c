#include "fish.h"
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
