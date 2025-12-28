#pragma once
#include "ringbuf.h"
#include "control_pi.h"
#include "plant_sim.h"
#include "fsm.h"

typedef struct {
    // Ring buffers dla CLI
    ringbuf_t rx, tx;
    
    // Stan regulatora
    control_pi_t controller;
    
    // Czujnik i symulacja
    plant_thermostat_t plant;
    
    // Zmienne procesu
    float setpoint;           // Żądana temperatura
    float measurement;        // Zmierzona temperatura z czujnika
    float control_output;     // Wyjście regulatora u
    
    // FSM
    fsm_t state_machine;
    
    // Telemetria
    uint32_t tick_count;      // Licznik iteracji
    uint32_t watchdog_counter; // Licznik do watchdoga
    uint32_t watchdog_timeout; // Ile ticków bez zmian = timeout
    uint32_t fault_count;
    float overshoot_max;
    
} thermostat_t;

void thermostat_init(thermostat_t* ts, float kp, float ki, float plant_alpha);

// tick to jedno wywołanie pętli sterowania (np. co 10 ms)
// Zawiera: odczyt czujnika, logika regulatora, aktualizacja FSM, telemetrię
void thermostat_tick(thermostat_t* ts);

void thermostat_rx_command(thermostat_t* ts, const char* cmd);
void thermostat_get_status(thermostat_t* ts, char* buf, size_t len);
