#pragma once
#include <stdint.h>

// Struktura reprezentująca symulowany obiekt termiczny (plant)
typedef struct {
    float temperature;  // Bieżąca temperatura obiektu
    float alpha;        // Parametr dynamiki termicznej - jak szybko obiekt reaguje na sterowanie
    float temp_limit_min, temp_limit_max; // Limity temperatury dla bezpieczeństwa
} plant_thermostat_t;

void plant_init(plant_thermostat_t* plant, float alpha);

// zwraca nową temperaturę po zastosowaniu wejścia
float plant_update(plant_thermostat_t* plant, float input);

// interfejs do bezpośredniego ustawienia temperatury czujnika
void plant_set_temperature(plant_thermostat_t* plant, float temp);
