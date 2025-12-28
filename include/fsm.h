#pragma once

typedef enum {
    STATE_INIT = 0,
    STATE_IDLE,
    STATE_RUNNING,
    STATE_FAULT,
    STATE_SAFE
} fsm_state_t;

typedef struct {
    // struktura przechowująca stan FSM
    fsm_state_t current_state;
    fsm_state_t next_state;
} fsm_t;

void fsm_init(fsm_t* fsm);

fsm_state_t fsm_next_state(fsm_t* fsm, int watchdog_ok, int limits_ok);

const char* fsm_state_name(fsm_state_t state);
