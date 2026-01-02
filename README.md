# Projekt zaliczeniowy - Termostat z regulatorem PI

Projekt implementuje cyfrowy termostat z regulatorem proporcjonalno-całkującym (PI). System jest symulowany i uruchamiany na hoście (PC).

## Architektura

System składa się z następujących komponentów:
- **Model obiektu (Plant)**: Symuluje obiekt termiczny o dynamice pierwszego rzędu.
- **Regulator PI**: Oblicza sygnał sterujący na podstawie błędu (różnicy między wartością zadaną a zmierzoną).
- **Maszyna stanów (FSM)**: Zarządza logiką systemu (stany: INIT, IDLE, RUNNING, FAULT, SAFE).
- **Interfejs CLI**: Umożliwia interakcję z użytkownikiem przez terminal (UART).
- **Bufor cykliczny (Ring Buffer)**: Zapewnia nieblokującą komunikację I/O.

## Wymagania

Do zbudowania i uruchomienia projektu potrzebny jest kompilator `gcc` i narzędzie `make`.

## Budowa

Aby zbudować projekt, wykonaj polecenie:
```bash
make build
```
Spowoduje to skompilowanie źródeł i umieszczenie pliku wykonywalnego `thermostat` w katalogu `build/`.

## Uruchomienie

Aby uruchomić symulację termostatu, wykonaj polecenie:
```bash
make run
```
Po uruchomieniu, program oczekuje na komendy z terminala.

## Interfejs użytkownika (CLI)

Dostępne komendy:
- `SET <wartość>`: Ustawia żądaną temperaturę (np. `SET 0.8`).
- `START`: Uruchamia regulator.
- `STOP`: Zatrzymuje regulator i przechodzi do stanu IDLE.
- `STATUS`: Wyświetla aktualny stan systemu i telemetrię.
- `RESET`: Resetuje system ze stanu FAULT lub SAFE do IDLE.

Co 50 kroków symulacji (około 0.5s), program automatycznie wypisuje dane telemetryczne:
```
T=<temp_zmierzona> SET=<temp_zadana> U=<sygnal_sterujacy> ST=<stan_FSM> OVH=<preregulowanie> TICKS=<liczba_krokow>
```
