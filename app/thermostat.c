#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "thermostat.h"

#define TS 0.01f  // Okres próbkowania - 10 ms
#define WD_TIMEOUT 500  // Watchdog - 500 ticków = 5 sekund

void thermostat_init(thermostat_t* ts, float kp, float ki, float plant_alpha) {
    ringbuf_init(&ts->rx);
    ringbuf_init(&ts->tx);
    
    control_pi_init(&ts->controller, kp, ki, 1.0f);
    plant_init(&ts->plant, plant_alpha);
    fsm_init(&ts->state_machine);
    
    // przejście INIT->IDLE - system gotów do pracy
    ts->state_machine.current = STATE_IDLE;
    ts->state_machine.next = STATE_IDLE;
    
    ts->setpoint = 0.0f;
    ts->measurement = 0.0f;
    ts->control_output = 0.0f;
    ts->tick_count = 0;
    ts->watchdog_counter = 0;
    ts->watchdog_timeout = WD_TIMEOUT;
    ts->fault_count = 0;
    ts->overshoot_max = 0.0f;
    
    // komunikat gotowości
    const char* greeting = "=== Thermostat Control System ===\r\nREADY\r\n";
    for (const char* p = greeting; *p; p++) {
        ringbuf_put(&ts->tx, (uint8_t)*p);
    }
}

void thermostat_tick(thermostat_t* ts) {
    // inkrementacja liczników
    ts->tick_count++;    
    ts->watchdog_counter++;
    
    // 1. Odczyt wartości z czujnika
    ts->measurement = plant_update(&ts->plant, ts->control_output);
    
    // 2. Logika regulatora temperatury
    if (ts->state_machine.current == STATE_RUNNING) {
        ts->control_output = control_pi_step(&ts->controller, ts->setpoint, ts->measurement, TS);
        
        // śledzenie overshoot'u
        // mierzymy o ile temperatura przekroczyła setpoint
        if (ts->measurement > ts->setpoint) {
            float overshoot = ts->measurement - ts->setpoint;
            if (overshoot > ts->overshoot_max) {
                ts->overshoot_max = overshoot;
            }
        }
    } else {
        // reset wyjścia i całki w trybach innych niż RUNNING
        ts->control_output = 0.0f;
        ts->controller.i_acc = 0.0f;
    }
    
    // 3.  FSM - sprawdzenie warunków przejścia
    // WHY: warunki dotyczą bezpieczeństwa (watchdog, limity temperatury)
    int limits_ok = (ts->measurement < 1.5f) && (ts->measurement > -1.5f);
    int watchdog_ok = (ts->watchdog_counter <= ts->watchdog_timeout);
    
    ts->state_machine.next = fsm_next_state(&ts->state_machine, watchdog_ok, limits_ok);
    ts->state_machine.current = ts->state_machine.next;
    
    // 4. Telemetria / Logi
    // wypisanie statusu co 50 ticków (przejrzystość logów)
    if (ts->tick_count % 50 == 0) {
        char status[128];
        thermostat_get_status(ts, status, sizeof(status));
        for (const char* p = status; *p; p++) {
            ringbuf_put(&ts->tx, (uint8_t)*p);
        }
    }
}

void thermostat_rx_command(thermostat_t* ts, const char* cmd) {
    // Obsługa komend tekstowych z interfejsu CLI
    
    if (strncmp(cmd, "SET ", 4) == 0) {
        float value = strtof(cmd + 4, NULL);
        ts->setpoint = value;
        
        char response[64];
        // wypisanie potwierdzenia ustawienia
        snprintf(response, sizeof(response), "OK SET=%.2f\r\n", value);
        for (const char* p = response; *p; p++) {
            ringbuf_put(&ts->tx, (uint8_t)*p);
        }
    } 
    else if (strncmp(cmd, "START", 5) == 0) {
        // START przechodzi do RUN, jeśli było w IDLE
        if (ts->state_machine.current == STATE_IDLE) {
            ts->state_machine.current = STATE_RUNNING;
            ts->watchdog_counter = 0;  // Reset watchdoga przy starcie
            ringbuf_put(&ts->tx, 'O');
            ringbuf_put(&ts->tx, 'K');
            ringbuf_put(&ts->tx, '\r');
            ringbuf_put(&ts->tx, '\n');
        } else {
            ringbuf_put(&ts->tx, 'E');
            ringbuf_put(&ts->tx, 'R');
            ringbuf_put(&ts->tx, 'R');
            ringbuf_put(&ts->tx, '\r');
            ringbuf_put(&ts->tx, '\n');
        }
    }
    else if (strncmp(cmd, "STOP", 4) == 0) {
        // STOP przechodzi do IDLE
        ts->state_machine.current = STATE_IDLE;
        ts->control_output = 0.0f;
        ringbuf_put(&ts->tx, 'O');
        ringbuf_put(&ts->tx, 'K');
        ringbuf_put(&ts->tx, '\r');
        ringbuf_put(&ts->tx, '\n');
    }
    else if (strncmp(cmd, "STATUS", 6) == 0) {
        char status[128];
        thermostat_get_status(ts, status, sizeof(status));
        for (const char* p = status; *p; p++) {
            ringbuf_put(&ts->tx, (uint8_t)*p);
        }
    }
    else if (strncmp(cmd, "RESET", 5) == 0) {
        // RESET pozwala wyjść z FAULT/SAFE - system musi wiedzieć, że problem jest naprawiony
        ts->state_machine.current = STATE_IDLE;
        ts->fault_count = 0;
        ts->overshoot_max = 0.0f;
        ringbuf_put(&ts->tx, 'O');
        ringbuf_put(&ts->tx, 'K');
        ringbuf_put(&ts->tx, '\r');
        ringbuf_put(&ts->tx, '\n');
    }
    else if (strncmp(cmd, "SENSOR", 6) == 0) {
        // interfejs wejściowy sensora - emuluje zmianę temperatury otoczenia
        // Pozwala testować odpowiedź regulatora na zewnętrzne zmiany temperatury
        float sensor_value = strtof(cmd + 7, NULL);  // Parse value after "SENSOR "
        plant_set_temperature(&ts->plant, sensor_value);
        
        char response[64];
        snprintf(response, sizeof(response), "OK SENSOR=%.2f\r\n", sensor_value);
        for (const char* p = response; *p; p++) {
            ringbuf_put(&ts->tx, (uint8_t)*p);
        }
    }
    else if (strncmp(cmd, "HELP", 4) == 0) {
        // dokumentacja dostępnych komend w systemie
        const char* help_text = "Commands: SET <val>, START, STOP, SENSOR <val>, STATUS, RESET, HELP\r\n";
        for (const char* p = help_text; *p; p++) {
            ringbuf_put(&ts->tx, (uint8_t)*p);
        }
    }
    else {
        ringbuf_put(&ts->tx, '?');
        ringbuf_put(&ts->tx, '\r');
        ringbuf_put(&ts->tx, '\n');
    }
}

void thermostat_get_status(thermostat_t* ts, char* buf, size_t len) {
    // telemetria w formacie tekstowym, łatwy do czytania dla operatora
    snprintf(buf, len,
        "T=%.2f SET=%.2f U=%.2f ST=%s OVH=%.3f TICKS=%lu\r\n",
        ts->measurement,
        ts->setpoint,
        ts->control_output,
        fsm_state_name(ts->state_machine.current),
        ts->overshoot_max,
        ts->tick_count
    );
}
