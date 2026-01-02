#pragma once
#include "ringbuf.h"
#include "control_pi.h"
#include "plant_sim.h"
#include "fsm.h"

// Struktura termostatu
typedef struct {
    // Ring buffer do komunikacji - wejście/wyjście do terminala
    ringbuf_t rx, tx;
    
    // Regulator PI
    control_pi_t controller;
    
    // Czujnik / Obiekt Fizyczny
    plant_thermostat_t plant;
    
    // Zmienne procesu
    float setpoint;           // Żądana temperatura
    float measurement;        // Zmierzona temperatura z czujnika
    float control_output;     // Wyjście regulatora PI
    
    // FSM
    fsm_t state_machine;
    
    // Telemetria
    int print_status;           // Flaga do drukowania statusu
    uint32_t tick_count;        // Licznik iteracji
    uint32_t watchdog_counter;  // Licznik watchdoga
    uint32_t watchdog_timeout;  // Ile ticków bez zmian = timeout
    uint32_t fault_count;       // Licznik błędów
    float overshoot_max;        // Statystyka overshoot'u
    
} thermostat_t;

void thermostat_init(thermostat_t* ts, float kp, float ki, float plant_alpha);

// Iteracja termostatu - wywoływana co okres TS (definiowany w thermostat.c)
void thermostat_tick(thermostat_t* ts);

// Obsługa komend tekstowych z interfejsu CLI
void thermostat_rx_command(thermostat_t* ts, const char* cmd);

// Pobranie statusu termostatu w formacie tekstowym
void thermostat_get_status(thermostat_t* ts, char* buf, size_t len);
