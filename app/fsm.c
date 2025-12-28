#include "fsm.h"

void fsm_init(fsm_t* fsm) {
    // inicjalizacja FSM do stanu początkowego
    fsm->current = STATE_INIT;
    fsm->next = STATE_INIT;
}

fsm_state_t fsm_next_state(fsm_t* fsm, int watchdog_ok, int limits_ok) {
    // zmiana stanu na podstawie warunków wejściowych
    // założenie FSM to proste przejścia stanów z mozliwością przejscia w stan bezpieczny w razie wystapienia problemu
    
    switch (fsm->current) {
        case STATE_INIT:
            // po inicjalizacji mozemy przejść jedynie do IDLE
            return STATE_IDLE;
        
        case STATE_IDLE:
            // z idle mozemy przejść do RUNNING lub FAULT
            // w zależności od tego czy limit jest przekroczony mozemy przejść do FAULT lub pozostać w IDLE, zmiana do run nastepuje z komendy operatora
            if (!limits_ok) return STATE_FAULT;
            return STATE_IDLE;
        
        case STATE_RUNNING:
            // z running mozemy przejść do IDLE, FAULT lub SAFE w zależności od warunków
            // jezeli limit jest przekroczony przechodzimy do FAULT
            // jezeli system nie odpowiada (!watchdog_ok) przechodzimy do SAFE
            // w przeciwnym wypadku pozostajemy w RUNNING
            if (!watchdog_ok) return STATE_SAFE;
            if (!limits_ok) return STATE_FAULT;
            return STATE_RUNNING;
        
        case STATE_FAULT:
            // fault wymaga interwencji operatora, nie mozna z niego wyjść automatycznie
            // implementacja wyjścia mogłaby skutkować zawieszeniem systemu w pętli FAULT->IDLE->FAULT
            return STATE_FAULT;
        
        case STATE_SAFE:
            // safe mozna opuścić tylko ręcznie - komenda operatora
            return STATE_SAFE;
        
        default:
            return STATE_SAFE;
    }
}

const char* fsm_state_name(fsm_state_t state) {
    switch (state) {
        case STATE_INIT:    return "INIT";
        case STATE_IDLE:    return "IDLE";
        case STATE_RUNNING: return "RUNNING";
        case STATE_FAULT:   return "FAULT";
        case STATE_SAFE:    return "SAFE";
        default:            return "?";
    }
}
