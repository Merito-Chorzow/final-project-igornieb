#include <stdlib.h>
#include <math.h>
#include "plant_sim.h"

void plant_init(plant_thermostat_t* plant, float alpha) {
    plant->temperature = 0.0f;
    plant->alpha = alpha;
}

float plant_update(plant_thermostat_t* plant, float input) {
    // Model liniowy termostatu: grzanie/chłodzenie wpływa na zmianę temperatury
    // input [-1, 1]: dodatnie = grzanie, ujemne = chłodzenie
    // alpha określa szybkość termiczną (mała alpha = powoli, duża alpha = szybko)
    //
    // Model: dT/dt = alpha * (input - 0.5*(T - T_ambient))
    // Uproszczenie bez otoczenia: dT/dt = alpha * input
    // To pozwala na asymptotyczną zbieżność do żądanej temperatury
    
    // Jeśli input > 0: grzejemy (T rośnie)
    // Jeśli input < 0: chłodzimy (T spada)
    // Jeśli input = 0: temperatura pozostaje stała (brak zmiany)
    
    plant->temperature = plant->temperature + plant->alpha * input;
    
    // Saturacja temperatury do zakresu [-10, 10]
    if (plant->temperature > 10.0f) plant->temperature = 10.0f;
    if (plant->temperature < -10.0f) plant->temperature = -10.0f;
    
    return plant->temperature;
}

void plant_set_temperature(plant_thermostat_t* plant, float temp) {
    // Do testów - bezpośrednie ustawienie temperatury
    plant->temperature = temp;
}
