#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "thermostat.h"

int main(void) {
    printf("=== Thermostat Control System ===\n");
    printf("PID: kp=0.5, ki=0.01, plant_alpha=0.1\n\n");
    
    thermostat_t ts;
    thermostat_init(&ts, 0.5f, 0.01f, 0.1f);
    
    // 1. Test step response w trybie RUN
    // Polecenie SET ustawia setpoint, START przechodzi w RUN
    printf("=== Test 1: Step Response (Set=0.7) ===\n");
    thermostat_rx_command(&ts, "SET 0.7");
    thermostat_rx_command(&ts, "START");
    
    // uruchamiamy 200 ticków (symulacja ~2 sekund przy ts=10ms)
    for (int i = 0; i < 200; i++) {
        thermostat_tick(&ts);
        
        // wypisujemy co 20 ticków, aby zobaczyć progres
        if (i % 20 == 0) {
            printf("[%3d] T=%6.3f SET=%6.3f U=%6.3f OVH=%6.3f STATE=%s\n",
                   i, ts.measurement, ts.setpoint, ts.control_output, 
                   ts.overshoot_max, fsm_state_name(ts.state_machine.current));
        }
    }
    
    // STOP zatrzymuje regulację
    thermostat_rx_command(&ts, "STOP");
    printf("\nStopped. Final overshoot: %.3f\n\n", ts.overshoot_max);
    
    // 2. Test zmiany setpointa
    printf("=== Test 2: Multiple Setpoints ===\n");
    thermostat_init(&ts, 0.5f, 0.01f, 0.15f);  // Szybsza roślina
    
    float setpoints[] = {0.3f, 0.6f, 0.9f, 0.5f};
    for (int sp = 0; sp < 4; sp++) {
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "SET %.1f", setpoints[sp]);
        thermostat_rx_command(&ts, cmd);
        thermostat_rx_command(&ts, "START");
        
        printf("\nSetpoint %.1f:\n", setpoints[sp]);
        for (int i = 0; i < 150; i++) {
            thermostat_tick(&ts);
            if (i % 30 == 0) {
                printf("  [%3d] T=%.3f U=%.3f\n", i, ts.measurement, ts.control_output);
            }
        }
        thermostat_rx_command(&ts, "STOP");
    }
    
    // 3. Test symulacji awarii (watchdog timeout)
    printf("\n=== Test 3: Watchdog Timeout ===\n");
    thermostat_init(&ts, 0.5f, 0.01f, 0.1f);
    thermostat_rx_command(&ts, "SET 0.5");
    thermostat_rx_command(&ts, "START");
    
    printf("Running normally for 50 ticks...\n");
    for (int i = 0; i < 50; i++) {
        thermostat_tick(&ts);
    }
    printf("State: %s, Temp: %.3f\n", fsm_state_name(ts.state_machine.current), ts.measurement);
    
    // WHY: symulujemy "zawieszenie" systemu - nie zwiększamy watchdog_counter
    printf("Simulating system hang (skipping ticks for 150 iterations)...\n");
    for (int i = 0; i < 150; i++) {
        // brak wywołania thermostat_tick - watchdog_counter się nie zwiększa
    }
    
    printf("After hang simulation, calling STATUS:\n");
    thermostat_rx_command(&ts, "STATUS");
    
    // 4. Test recovery z FAULT/SAFE stanu
    printf("\n=== Test 4: RESET Command ===\n");
    thermostat_rx_command(&ts, "RESET");
    printf("State after RESET: %s\n", fsm_state_name(ts.state_machine.current));
    thermostat_rx_command(&ts, "STATUS");
    
    printf("\n=== TX Buffer Output ===\n");
    uint8_t byte;
    while (ringbuf_get(&ts.tx, &byte)) {
        putchar((char)byte);
    }
    
    printf("\n\n=== All Tests Completed ===\n");
    return 0;
}
