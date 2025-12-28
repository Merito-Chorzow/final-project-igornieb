#pragma once
#include <stdint.h>

typedef struct {
    float temp;         // Bieżąca temperatura
    float alpha;        // Parametr dynamiki termostatu: 0 < alpha < 1
    // WHY: alpha reprezentuje stosunek Ts/tau (gdzie tau to stała czasowa)
    // Wyższa alpha = szybsze dopasowanie się temperatury do wejścia
    // temp[k+1] = temp[k] + alpha * (input - temp[k])
} plant_thermostat_t;

void plant_init(plant_thermostat_t* plant, float alpha);

// update (nie 'simulate') bo to rzeczywisty model obiektu
// input: sterowanie z regulatora u [-1, 1]
// out: zmierzoną temperaturę z szumem
float plant_update(plant_thermostat_t* plant, float input);

// interfejs do bezpośredniego ustawienia temperatury czujnika
// Emuluje operator ustawiający temperaturę (np. zmiana otoczenia)
// Używane w testach do symulacji warunków
void plant_set_temperature(plant_thermostat_t* plant, float temp);
