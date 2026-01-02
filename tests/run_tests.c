#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "thermostat.h"

int main(void) {
    printf("=== Thermostat Control System - Test Suite ===\n");
    printf("PID: kp=1.2, ki=0.15, plant_alpha=0.6\n\n");
    
    thermostat_t ts;
    thermostat_init(&ts, 1.2f, 0.15f, 0.6f);

    // Test 1. Ustawienie temperatury i uruchomienie regulacji
    printf("=== Test setpoint ===\n");
    thermostat_rx_command(&ts, "SET 5.0");
    thermostat_rx_command(&ts, "START");
    
    // uruchamiamy 440 ticków (symulacja 4.4 sekund)
    for (int i = 0; i < 500; i++) {
        thermostat_tick(&ts);
        
        // Wypisanie statusu co 40 ticków (debugging)
        if (i % 40 == 0) {
            printf("[%3d] T=%6.3f SET=%6.3f U=%6.3f OVH=%6.3f STATE=%s\n",
                   i, ts.measurement, ts.setpoint, ts.control_output, 
                   ts.overshoot_max, fsm_state_name(ts.state_machine.current_state));
        }
    }
    
    // STOP zatrzymuje regulację
    thermostat_rx_command(&ts, "STOP");
    printf("\nStopped. Final overshoot: %.3f\n\n", ts.overshoot_max);
    
    // 2. Test zmiany setpointa w trakcie pracy
    printf("=== Test 2: Multiple Setpoints (Heating & Cooling) ===\n");
    thermostat_init(&ts, 1.2f, 0.15f, 0.6f);
    
    float setpoints[] = {-2.0f, 4.0f, -3.0f, 1.0f};
    for (int sp = 0; sp < 4; sp++) {
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "SET %.1f", setpoints[sp]);
        thermostat_rx_command(&ts, cmd);
        thermostat_rx_command(&ts, "START");
        
        printf("\nSetpoint %.1f:\n", setpoints[sp]);
        // Dłuższy czas na stabilizację
        for (int i = 0; i < 250; i++) {
            thermostat_tick(&ts);
            if (i % 50 == 0) {
                printf("  [%3d] T=%.3f U=%.3f\n", i, ts.measurement, ts.control_output);
            }
        }
        thermostat_rx_command(&ts, "STOP");
    }
    
    // 3. Test symulacji awarii (watchdog timeout)
    printf("\n=== Test 3: Watchdog Timeout ===\n");
    thermostat_init(&ts, 1.2f, 0.15f, 0.6f);
    thermostat_rx_command(&ts, "SET 0.5");
    thermostat_rx_command(&ts, "START");
    
    printf("Running normally for 50 ticks...\n");
    for (int i = 0; i < 50; i++) {
        thermostat_tick(&ts);
    }
    printf("State: %s, Temp: %.3f\n", fsm_state_name(ts.state_machine.current_state), ts.measurement);
    
    // SYMULACJA ZAWIESZENIA: nadal wywoływujemy tick(), ale bez przetwarzania komend
    // W ten sposób watchdog_counter rośnie, ale system "nie odpowiada na komendy"
    printf("Simulating system hang (running %d more ticks without new commands)...\n", 600);
    for (int i = 0; i < 600; i++) {
        thermostat_tick(&ts);
        if (i == ts.watchdog_timeout) {
            printf("  [tick %d] Watchdog timeout triggered! State should change to SAFE\n", 50 + i);
        }
    }
    
    printf("After hang simulation, checking status:\n");
    printf("State: %s (should be SAFE), Temp: %.3f, U: %.3f\n", 
           fsm_state_name(ts.state_machine.current_state), ts.measurement, ts.control_output);
    thermostat_rx_command(&ts, "STATUS");
    
    // 4. Test recovery z SAFE stanu
    printf("\n=== Test 4: RESET Command (Recovery from SAFE) ===\n");
    printf("State before RESET: %s\n", fsm_state_name(ts.state_machine.current_state));
    thermostat_rx_command(&ts, "RESET");
    printf("State after RESET: %s (should be IDLE)\n", fsm_state_name(ts.state_machine.current_state));
    thermostat_rx_command(&ts, "STATUS");
    
    printf("\n=== TX Buffer Output ===\n");
    uint8_t byte;
    while (ringbuf_get(&ts.tx, &byte)) {
        putchar((char)byte);
    }
    
    printf("\n\n=== All Tests Completed ===\n");
    return 0;
}
