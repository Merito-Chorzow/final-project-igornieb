#pragma once

// Dostępne stany FSM
typedef enum {
    STATE_INIT = 0,
    STATE_IDLE,
    STATE_RUNNING,
    STATE_FAULT,
    STATE_SAFE
} fsm_state_t;

// Obiekt FSM
typedef struct {
    fsm_state_t current_state;
    fsm_state_t next_state;
} fsm_t;

// Inicjalizacja FSM do stanu początkowego
void fsm_init(fsm_t* fsm);

// Ustawienie następnego stanu na podstawie warunków wejściowych
fsm_state_t fsm_next_state(fsm_t* fsm, int watchdog_ok, int limits_ok);

const char* fsm_state_name(fsm_state_t state);
