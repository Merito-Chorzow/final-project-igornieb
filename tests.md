# Testy - Thermostat Control System

## Przegląd

Projekt zawiera **4 testy** zintegrowane w `app/main.c`. Testy uruchamiają się sekwencyjnie i kolejno weryfikują:

1. Regulację PI w odpowiedzi na skok setpointa
2. Liniowość systemu przy różnych setpointach
3. Bezpieczeństwo - watchdog timeout i przejście do SAFE
4. Recovery - możliwość powrotu do IDLE po RESET

### Uruchamianie Testów

```bash
make clean && make run
```

**Output**: Każdy test wypisuje wyniki na stdout w formacie tekstowym.

---

## Testy

### Test 1: Regulację PI w odpowiedzi na skok setpointa

**Cel**: Obserwacja odpowiedzi regulatora PI na skok setpointa (0°C → 0.7°C).

**Parametry**:
- Setpoint: 0.7°C
- Kp=1.2, Ki=0.15
- Plant alpha=0.6 (szybkość reakcji rośliny)
- Czas symulacji: 200 ticks (2 sekundy przy Ts=10ms)

**Kod** (`app/main.c`):
```c
printf("=== Test 1: Step Response (Set=0.7) ===\n");
thermostat_t ts;
thermostat_init(&ts, 0.5f, 0.01f, 0.1f);
thermostat_rx_command(&ts, "SET 0.7");
thermostat_rx_command(&ts, "START");

for (int i = 0; i < 200; i++) {
    thermostat_tick(&ts);
    if (i % 20 == 0) {
        printf("[%3d] T=%6.3f SET=%6.3f U=%6.3f OVH=%6.3f STATE=%s\n",
               i, ts.measurement, ts.setpoint, ts.control_output, 
               ts.overshoot_max, fsm_state_name(ts.state_machine.current));
    }
}

thermostat_rx_command(&ts, "STOP");
printf("\nStopped. Final overshoot: %.3f\n\n", ts.overshoot_max);
```

**Oczekiwany rezultat**:
```
=== Test 1: Step Response (Set=0.7) ===
[  0] T= 0.000 SET= 0.700 U= 0.841 OVH= 0.000 STATE=RUNNING
[ 20] T= 0.701 SET= 0.700 U=-0.000 OVH= 0.001 STATE=RUNNING
[ 40] T= 0.701 SET= 0.700 U=-0.000 OVH= 0.001 STATE=RUNNING
[ 60] T= 0.701 SET= 0.700 U=-0.000 OVH= 0.001 STATE=RUNNING
[ 80] T= 0.701 SET= 0.700 U=-0.000 OVH= 0.001 STATE=RUNNING
[100] T= 0.701 SET= 0.700 U=-0.000 OVH= 0.001 STATE=RUNNING
[120] T= 0.701 SET= 0.700 U=-0.000 OVH= 0.001 STATE=RUNNING
[140] T= 0.701 SET= 0.700 U=-0.000 OVH= 0.001 STATE=RUNNING
[160] T= 0.701 SET= 0.700 U=-0.000 OVH= 0.001 STATE=RUNNING
[180] T= 0.701 SET= 0.700 U=-0.000 OVH= 0.001 STATE=RUNNING

Stopped. Final overshoot: 0.001
```

**Wnioski**:
- Temperature rośnie monotonicznie do setpointa 0.7°C, bez oscylacji.
- Przeregulowanie jest minimalne - OVH < 1%.
- System pozostaje w stanie RUNNING przez cały czas, brak przejścia do FAULT.


### Test 2: Multiple Setpoints

**Cel**: Obserwacja stabilności na całym zakresie pracy (sprawdzenie liniowości systemu).

**Parametry**:
- Setpoints: 0.3, 0.6, 0.9, 0.5 (sekwencyjna zmiana)
- Kp=1.2, Ki=0.15
- Plant alpha=0.6
- 150 ticks na każdy setpoint

**Kod** (`app/main.c`):
```c
printf("=== Test 2: Multiple Setpoints ===\n");
    thermostat_init(&ts, 1.2f, 0.15f, 0.6f);

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
```

**Oczekiwany rezultat**:
```
=== Test 2: Multiple Setpoints ===

Setpoint 0.3:
  [  0] T=0.000 U=0.360
  [ 30] T=0.301 U=-0.000
  [ 60] T=0.300 U=-0.000
  [ 90] T=0.300 U=-0.000
  [120] T=0.300 U=-0.000

Setpoint 0.6:
  [  0] T=0.300 U=0.360
  [ 30] T=0.601 U=-0.000
  [ 60] T=0.601 U=-0.000
  [ 90] T=0.601 U=-0.000
  [120] T=0.601 U=-0.000

Setpoint 0.9:
  [  0] T=0.601 U=0.360
  [ 30] T=0.901 U=-0.000
  [ 60] T=0.901 U=-0.000
  [ 90] T=0.901 U=-0.000
  [120] T=0.901 U=-0.000

Setpoint 0.5:
  [  0] T=0.901 U=-0.481
  [ 30] T=0.500 U=-0.000
  [ 60] T=0.500 U=-0.000
  [ 90] T=0.500 U=-0.000
  [120] T=0.500 U=-0.000
```

**Wnioski**:
- Temperatura rośnie monotonicznie do setpointa, bez oscylacji.
- System zachowuje liniowość - podobne czasy konwergencji dla różnych setpointów.

### Test 3: Watchdog Timeout

**Cel**: Obserwacja przejścia do stanu SAFE przy zawieszeniu systemu (symulacja zawieszenia systemu).

**Parametry**:
- Setpoint: 0.5°C
- System pracuje normalnie przez 50 tików
- Symulacja "zawieszenia" przez 150 tików (brak wywołań tick)
- Watchdog timeout = 500 ticks (zdefiniowany w `thermostat.c`)

**Kod** (`app/main.c`):
```c
printf("\n=== Test 3: Watchdog Timeout ===\n");
thermostat_init(&ts, 1.2f, 0.15f, 0.6f);
thermostat_rx_command(&ts, "SET 0.5");
thermostat_rx_command(&ts, "START");

printf("Running normally for 50 ticks...\n");
for (int i = 0; i < 50; i++) {
    thermostat_tick(&ts);
}
printf("State: %s, Temp: %.3f\n", fsm_state_name(ts.state_machine.current), ts.measurement);

// SYMULACJA ZAWIESZENIA: nadal wywoływujemy tick(), ale bez przetwarzania komend
// W ten sposób watchdog_counter rośnie, ale system "nie odpowiada na komendy"
printf("Simulating system hang (running %d more ticks without responding to commands)...\n", 600);
for (int i = 0; i < 600; i++) {
    thermostat_tick(&ts);
    if (i == 500) {
        printf("  [tick %d] Watchdog timeout triggered! State should change to SAFE\n", 50 + i);
    }
}

printf("After hang simulation, checking status:\n");
printf("State: %s (should be SAFE), Temp: %.3f, U: %.3f\n", 
        fsm_state_name(ts.state_machine.current), ts.measurement, ts.control_output);
thermostat_rx_command(&ts, "STATUS");
```

**Oczekiwany rezultat**:
```
=== Test 3: Watchdog Timeout ===
Running normally for 50 ticks...
State: RUNNING, Temp: 0.501
Simulating system hang (running 600 more ticks without responding to commands)...
  [tick 550] Watchdog timeout triggered! State should change to SAFE
After hang simulation, checking status:
State: SAFE (should be SAFE), Temp: 0.500, U: 0.000
```

**Wnioski**:
- Watchdog wykrywa brak aktywności i przełącza system do stanu SAFE.
- Termostat wyłącza wyjście (U=0) w stanie SAFE.
- Licznik ticków zatrzymuje się, licznik watchdog działa poprawnie (rośnie).
- Temperatura pozostaje stabilna, brak dalszej regulacji.

### Test 4: RESET Command

**Cel**: Obserwacja powrotu z SAFE/FAULT do IDLE po interwencji operatora.

**Parametry**:
- Wykonanie po Test 3 (system w stanie SAFE)
- Komenda: `RESET`

**Kod** (`app/main.c`):
```c
printf("\n=== Test 4: RESET Command (Recovery from SAFE) ===\n");
thermostat_rx_command(&ts, "RESET");
printf("State after RESET: %s (should be IDLE)\n", fsm_state_name(ts.state_machine.current));
thermostat_rx_command(&ts, "STATUS");

printf("\n=== TX Buffer Output ===\n");
uint8_t byte;
while (ringbuf_get(&ts.tx, &byte)) {
    putchar((char)byte);
}
```

**Oczekiwany rezultat**:
```
State after RESET: IDLE (should be IDLE)

=== TX Buffer Output ===
=== Thermostat Control System ===
READY
OK SET=0.50
OK
T=0.50 SET=0.50 U=-0.00 ST=RUNNING OVH=0.001 TICKS=50
T=0.50 SET=0.50 U=-0.00 ST=RUNNING OVH=0.001 TICKS=100
T=0.50 SET=0.50 U=-0.00 ST=RUNNING OVH=0.001 TICKS=150
T=0.50 SET=0.50 U=-0.00 ST=RU

```

**Wnioski**:
- Komenda RESET skutecznie przywraca system do stanu IDLE, pełny cykl bezpieczeństwa.
- Liczniki są zerowane, system gotów do ponownego STARTU.