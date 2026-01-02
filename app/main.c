#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "thermostat.h"

// Funkcja pomocnicza do wypisywania odpowiedzi z termostatu
void print_thermostat_response(thermostat_t* ts) {
    uint8_t byte;
    while (ringbuf_get(&ts->tx, &byte)) {
        putchar((char)byte);
    }
}

int main(void) {
    printf("=== Thermostat Simulator ===\n");
    printf("Available commands: SET <val>, START, STOP, STATUS, RESET, HELP, TICK <n>, EXIT\n\n");
    printf("Additional simulation commands:\n");
    printf("  TICK <n> - Run simulation for n ticks\n");
    printf("  SENSOR <val> - Set temperature (overrides current temperature)\n");
    printf("  EXIT - Exit simulator\n");

    
    thermostat_t ts;
    // Ustrawienie paramentów regulatora i symulacji
    thermostat_init(&ts, 1.2f, 0.15f, 0.6f);

    print_thermostat_response(&ts);

    char line_buffer[128];

    // Pętla główna
    // Umożliwia symulaję upływu czasu i interakcję z termostatem
    while (1) {
        // Zasugerowanie użytkownikowi wpisania komendy
        printf("> ");
        fflush(stdout);

        // Czytaj komendę od użytkownika
        if (fgets(line_buffer, sizeof(line_buffer), stdin) == NULL) {
            break; // Wyjście przy EOF/bledzie 
        }
        // Trzeba usunąć znak nowej linii z końca komendy
        line_buffer[strcspn(line_buffer, "\r\n")] = 0;

        // Obsługa komend specjalnych/symulacyjnych
        // EXIT - wyjście z symulatora
        if (strncmp(line_buffer, "EXIT", 4) == 0) {
            printf("Exiting simulator.\n");
            break;
        }

        // TICK - symulacja upływu czasu
        if (strncmp(line_buffer, "TICK", 4) == 0) {
            int ticks_to_run = 1;
            // Walidacja liczby ticków
            if (strlen(line_buffer) > 4) {
                ticks_to_run = atoi(line_buffer + 5);
            }
            if (ticks_to_run > 0) {
                printf("--- Running %d simulation tick(s) ---\n", ticks_to_run);
                for (int i = 0; i < ticks_to_run; i++) {
                    thermostat_tick(&ts);
                }
                // Automatycznie wyświetl status po tickach, aby zobaczyć efekt
                thermostat_rx_command(&ts, "STATUS");
            }
        } else {
            // Obsługa komend termostatus
            thermostat_rx_command(&ts, line_buffer);
        }

        // Odpowiedź z termostatu
        print_thermostat_response(&ts);
    }

    printf("\n=== Simulation Finished ===\n");
    return 0;
}
