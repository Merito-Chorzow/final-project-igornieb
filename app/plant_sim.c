#include <stdlib.h>
#include <math.h>
#include "plant_sim.h"

// stałe ziarno PRNGa, aby testy były powtarzalne
static uint32_t rng_state = 12345;

static float prng_uniform(void) {
    // WHY: prosty LCG (Linear Congruential Generator)
    // Wystarczająco dobry dla symulacji szumu, deterministyczny
    rng_state = (rng_state * 1103515245 + 12345) & 0x7fffffff;
    return (float)rng_state / 0x7fffffff;
}

void plant_init(plant_thermostat_t* plant, float alpha) {
    plant->temp = 0.0f;
    plant->alpha = alpha;
}

float plant_update(plant_thermostat_t* plant, float input) {
    // model 1-rzędu (first-order): temp[k+1] = temp[k] + alpha * (input - temp[k])
    // Model RC (rezystancja-pojemność) termostat:
    // - alpha duże = mała inercja (szybka zmiana)
    // - alpha małe = duża inercja (powoli się zmienia)
    plant->temp = plant->temp + plant->alpha * (input - plant->temp);
    
    // WHY: dodajemy szum pomiarowy
    // Czujniki w rzeczywistości nigdy nie są idealnie dokładne
    // Szum o amplitudzie ~0.02 symuluje rzeczywisty czujnik
    float noise = (prng_uniform() - 0.5f) * 0.02f;
    
    return plant->temp + noise;
}

void plant_set_temperature(plant_thermostat_t* plant, float temp) {
    // Do testów - bezpośrednie ustawienie temperatury
    plant->temp = temp;
}
