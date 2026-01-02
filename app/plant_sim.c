#include <stdlib.h>
#include <math.h>
#include "plant_sim.h"

void plant_init(plant_thermostat_t* plant, float alpha) {
    plant->temperature = 0.0f;
    plant->alpha = alpha;
    plant->temp_limit_min = -10.0f;
    plant->temp_limit_max = 10.0f;
}

float plant_update(plant_thermostat_t* plant, float input) {
    // Model symulujący nagrzewanie/chłodzenie obiektu (plant)
    // Parametry:
    // input [-1, 1]: dodatnie = grzanie, ujemne = chłodzenie, wartość reguluje szybkość zmiany temperatury przez termostat
    // alpha: określa szybkość zmiany temperatury obiektu
    
    // Wzór: T[k+1] = T[k] + alpha * input
    // input > 0: grzejemy (T rośnie)
    // input < 0: chłodzimy (T spada)
    // input = 0: temperatura pozostaje stała (brak zmiany)
    plant->temperature = plant->temperature + plant->alpha * input;
    
    if (plant->temperature > plant->temp_limit_max) plant->temperature = plant->temp_limit_max;
    if (plant->temperature < plant->temp_limit_min) plant->temperature = plant->temp_limit_min;
    
    return plant->temperature;
}

void plant_set_temperature(plant_thermostat_t* plant, float temp) {
    // Do testów - bezpośrednie ustawienie temperatury symulowanego obiektu
    plant->temperature = temp;
}
