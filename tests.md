# Testy - Thermostat Control System

## Przegląd

Projekt zawiera **4 główne scenariusze testowe** zintegrowane w `app/main.c`. Testy uruchamiają się sekwencyjnie i weryfikują kluczowe aspekty działania termostatu:

1.  **Reakcja na duży skok temperatury**: Sprawdzenie wydajności regulatora przy dużym błędzie (zadanie ogrzania systemu).
2.  **Praca w szerokim zakresie**: Testowanie stabilności i odpowiedzi regulatora przy zadaniach grzania i chłodzenia.
3.  **Mechanizmy bezpieczeństwa**: Weryfikacja, czy watchdog poprawnie wykrywa brak aktywności i przełącza system w stan `SAFE`.
4.  **Powrót ze stanu awaryjnego**: Sprawdzenie, czy komenda `RESET` prawidłowo przywraca system do normalnej pracy.

### Uruchamianie Testów

Testy są zautomatyzowane. Wystarczy skompilować i uruchomić program:
```bash
make clean && make run
```
Każdy test wypisuje swoje wyniki na standardowe wyjście, co pozwala na łatwą analizę.

---

## Szczegółowe Scenariusze Testowe

### Test 1: Reakcja na duży skok (grzanie)

*   **Cel**: Obserwacja odpowiedzi regulatora PI na duży, dodatni skok wartości zadanej (od 0.0 do 5.0). Weryfikacja czasu narastania i wielkości przeregulowania (`overshoot`).
*   **Kroki**:
    1.  Inicjalizacja termostatu.
    2.  Ustawienie `setpoint` na `5.0`.
    3.  Uruchomienie regulatora (`START`).
    4.  Symulacja przez `400` ticków (4 sekundy).
*   **Oczekiwany rezultat**: Temperatura powinna płynnie wzrosnąć do wartości zadanej. Dopuszczalne jest niewielkie przeregulowanie, które regulator powinien następnie skorygować. System powinien pozostać stabilny w stanie `RUNNING`.

### Test 2: Praca w szerokim zakresie (grzanie i chłodzenie)

*   **Cel**: Sprawdzenie, jak regulator radzi sobie z sekwencją różnych wartości zadanych, wymagających zarówno grzania, jak i chłodzenia. Testuje to uniwersalność i stabilność algorytmu PI.
*   **Kroki**:
    1.  Reinicjalizacja termostatu.
    2.  Sekwencyjne ustawianie `setpoint` na wartości: `-2.0`, `4.0`, `-3.0`, `1.0`.
    3.  Dla każdej wartości zadanej symulacja jest prowadzona przez `250` ticków (2.5 sekundy), aby umożliwić stabilizację.
*   **Oczekiwany rezultat**: Dla każdej wartości zadanej temperatura powinna zbiegać do celu. Przy zmianach znaku `setpoint` (np. z 4.0 na -3.0), sterowanie `U` powinno zmienić znak, aktywując "chłodzenie".

### Test 3: Zadziałanie Watchdoga

*   **Cel**: Weryfikacja, czy mechanizm watchdoga poprawnie identyfikuje "zawieszenie" systemu i przełącza go w bezpieczny stan.
*   **Kroki**:
    1.  Uruchomienie regulatora w normalnym trybie.
    2.  Symulacja pracy przez 50 ticków.
    3.  Symulacja "zawieszenia" przez dalsze `600` ticków, podczas których `thermostat_tick()` jest wywoływany, ale nie są przetwarzane żadne komendy (co uniemożliwia reset licznika watchdoga).
*   **Oczekiwany rezultat**: Po upływie `watchdog_timeout` (500 ticków), maszyna stanów powinna automatycznie przejść ze stanu `RUNNING` do `STATE_SAFE`. W stanie `SAFE` sterowanie `U` powinno zostać wyzerowane, zatrzymując aktywne grzanie/chłodzenie.

### Test 4: Powrót ze stanu awaryjnego (RESET)

*   **Cel**: Sprawdzenie, czy system można przywrócić do normalnej pracy ze stanu `SAFE` za pomocą interwencji operatora.
*   **Kroki**:
    1.  System znajduje się w stanie `SAFE` po Teście 3.
    2.  Wysłanie komendy `RESET`.
*   **Oczekiwany rezultat**: Po otrzymaniu komendy `RESET`, maszyna stanów powinna natychmiast przejść ze stanu `SAFE` do `STATE_IDLE`. System powinien być gotowy do przyjęcia nowych komend, a liczniki błędów i przeregulowania wyzerowane.

## Rzeczywiste wyniki testów (logi)

Poniżej znajdują się kompletne logi wygenerowane przez ostatnie pomyślne uruchomienie testów. Stanowią one wzorzec, do którego można porównywać przyszłe wyniki.

```log
=== Thermostat Control System ===
PID: kp=1.2, ki=0.15, plant_alpha=0.6

=== Test 1: Step Response (Heating, Set=5.0) ===
[  0] T= 0.000 SET= 5.000 U= 1.000 OVH= 0.000 STATE=RUNNING
[ 40] T= 5.028 SET= 5.000 U=-0.000 OVH= 0.029 STATE=RUNNING
[ 80] T= 5.027 SET= 5.000 U=-0.000 OVH= 0.029 STATE=RUNNING
[120] T= 5.026 SET= 5.000 U=-0.000 OVH= 0.029 STATE=RUNNING
[160] T= 5.024 SET= 5.000 U=-0.000 OVH= 0.029 STATE=RUNNING
[200] T= 5.023 SET= 5.000 U=-0.000 OVH= 0.029 STATE=RUNNING
[240] T= 5.022 SET= 5.000 U=-0.000 OVH= 0.029 STATE=RUNNING
[280] T= 5.021 SET= 5.000 U=-0.000 OVH= 0.029 STATE=RUNNING
[320] T= 5.020 SET= 5.000 U=-0.000 OVH= 0.029 STATE=RUNNING
[360] T= 5.019 SET= 5.000 U=-0.000 OVH= 0.029 STATE=RUNNING

Stopped. Final overshoot: 0.029

=== Test 2: Multiple Setpoints (Heating & Cooling) ===

Setpoint -2.0:
  [  0] T=0.000 U=-1.000
  [ 50] T=-2.005 U=0.000
  [100] T=-2.005 U=0.000
  [150] T=-2.005 U=0.000
  [200] T=-2.004 U=0.000

Setpoint 4.0:
  [  0] T=-2.004 U=1.000
  [ 50] T=4.036 U=-0.000
  [100] T=4.033 U=-0.000
  [150] T=4.031 U=-0.000
  [200] T=4.030 U=-0.000

Setpoint -3.0:
  [  0] T=4.028 U=-1.000
  [ 50] T=-3.027 U=0.000
  [100] T=-3.025 U=0.000
  [150] T=-3.024 U=0.000
  [200] T=-3.022 U=0.000

Setpoint 1.0:
  [  0] T=-3.021 U=1.000
  [ 50] T=0.999 U=0.000
  [100] T=0.999 U=0.000
  [150] T=0.999 U=0.000
  [200] T=0.999 U=0.000

=== Test 3: Watchdog Timeout ===
Running normally for 50 ticks...
State: RUNNING, Temp: 0.501
Simulating system hang (running 600 more ticks without responding to commands)...
  [tick 550] Watchdog timeout triggered! State should change to SAFE
After hang simulation, checking status:
State: SAFE (should be SAFE), Temp: 0.500, U: 0.000

=== Test 4: RESET Command (Recovery from SAFE) ===
State after RESET: IDLE (should be IDLE)
```

---
