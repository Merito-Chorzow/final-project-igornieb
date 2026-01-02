# Termostat z regulatorem PI

## 1. Diagram Blokowy

Interfejs użytkownika komunikuje się z warstwą logiczną termostatu z użyciem CLI.
Architektura zapewnia modularność i separację odpowiedzialności między komponentami. Umożliwia to podmianę poszczególnych modułów (np. modelu obiektu fizycznego) bez wpływu na resztę systemu.

### Komponenty:
-   **Interfejs Użytkownika (CLI):** Pętla w `main.c`, która odczytuje komendy tekstowe od użytkownika i przekazuje je do logiki termostatu.
-   **Logika Termostatu:** Główny moduł (`thermostat.c`), który koordynuje pracę pozostałych komponentów. Wywołuje cykle symulacji (`thermostat_tick`), przetwarza komendy i zarządza stanem.
-   **Maszyna Stanów (FSM):** Implementuje cykl życia systemu (stany: INIT, IDLE, RUNNING, FAULT, SAFE) i definiuje warunki przejść między nimi, zapewniając przewidywalne i bezpieczne działanie.
-   **Regulator PI:** Moduł (`control_pi.c`) implementujący algorytm regulacji proporcjonalno-całkującej. Jego zadaniem jest obliczenie sygnału sterującego na podstawie zadanej i zmierzonej temperatury.
-   **Model Obiektu Fizycznego:** Symulator (`plant_sim.c`), który emuluje zachowanie rzeczywistego systemu termicznego. Pełni podwójną rolę:
    -   **Czujnika:** Zwraca aktualną, symulowaną temperaturę.
    -   **Aktuatora:** Zmienia swoją temperaturę w odpowiedzi na sygnał sterujący z regulatora PI (uwzględniając dynamikę cieplną - współczynnik alfa).

## 2. Maszyna stanów (FSM)

Maszyna stanów zarządza trybem pracy termostatu.

### Opis Stanów:
-   **IDLE:** System jest gotowy do pracy, ale pętla regulacji jest nieaktywna. Oczekuje na komendę `START`.
-   **RUNNING:** Główny stan operacyjny. Regulator PI jest aktywny i steruje temperaturą obiektu.
-   **FAULT:** Stan błędu, aktywowany po przekroczeniu bezpiecznych limitów temperatury. Wymaga interwencji operatora (komenda `RESET`).
-   **SAFE:** Stan bezpieczny, aktywowany w przypadku braku odpowiedzi systemu (timeout watchdoga). Wymaga interwencji operatora (komenda `RESET`).

## 3. Protokół Komunikacyjny (CLI)

Komunikacja z termostatem odbywa się za pomocą prostego protokołu tekstowego opartego o wiersz poleceń. Każda komenda musi być zakończona znakiem nowej linii.

### Komendy Systemowe:
-   `SET <wartość>`
    -   Opis: Ustawia docelową temperaturę (setpoint).
    -   Argument `<wartość>`: Liczba zmiennoprzecinkowa (np. `5.0`, `-2.5`).
    -   Przykład: `SET 21.5`

-   `START`
    -   Opis: Uruchamia proces regulacji. Przechodzi ze stanu `IDLE` do `RUNNING`.

-   `STOP`
    -   Opis: Zatrzymuje proces regulacji. Przechodzi ze stanu `RUNNING` do `IDLE`.

-   `STATUS`
    -   Opis: Zwraca aktualny stan telemetryczny termostatu (temperatura, nastawa, sygnał sterujący, stan FSM, etc.).

-   `RESET`
    -   Opis: Resetuje system ze stanu `FAULT` lub `SAFE` do stanu `IDLE`.

-   `SENSOR <wartość>`
    -   Opis: Bezpośrednio ustawia temperaturę w emulowanym czujniku. Służy do testowania reakcji systemu na nagłe zmiany zewnętrzne.
    -   Przykład: `SENSOR 10.0`

-   `HELP`
    -   Opis: Wyświetla listę dostępnych komend.

### Komendy Symulatora (tylko w `main.c`):

Poniższe komendy służą do symulacji upływu czasu i interakcji z termostatem w środowisku testowym.

-   `TICK <n>`
    -   Opis: Wykonuje `<n>` cykli symulacji. Jeśli `n` nie jest podane, wykonuje 1 cykl.
    -   Przykład: `TICK 100`

-   `EXIT`
    -   Opis: Zamyka interaktywny symulator.
