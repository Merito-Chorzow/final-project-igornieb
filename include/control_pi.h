#pragma once
#include <stdint.h>

typedef struct {
    float kp;           // Proporcjonalne wzmocnienie
    float ki;           // Całkujące wzmocnienie
    float i_acc;        // Akumulator całki - musi być zachowywany między iteracjami
    float e_prev;       // Poprzedni błąd - potrzebny dla filtracji
    float i_limit;      // Anti-windup: limit całki zapobiega nagromadzeniu błędu
    float u_min, u_max; // Saturacja wyjścia - bez limitu układ wychodzi z zakresu fizycznego
} control_pi_t;

void control_pi_init(control_pi_t* pi, float kp, float ki, float i_limit);

// regulator proporcjonalno-całkujący (PI), zastosowany w celu utrzymania temperatury na zadanym poziomie (setpoint)
// funkcja zwraca wartość sterowania `u` w zakresie [-1, 1] dla grzałki, aby zmniejszyć różnicę między wartościami `setpoint` a `measurement`
float control_pi_step(control_pi_t* pi, float setpoint, float measurement, float ts);
