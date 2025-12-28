#pragma once
#include <stdint.h>

typedef struct {
    float kp;           // Proporcjonalne wzmocnienie
    float ki;           // Całkujące wzmocnienie
    float integral_accumulator; // Akumulator całki
    float previous_error;     // Poprzedni błąd
    float integral_limit;     // Anti-windup: limit całki
    float u_min, u_max; // Saturacja wyjścia - bez limitu układ wychodzi z zakresu fizycznego
} control_pi_t;

void control_pi_init(control_pi_t* pi, float kp, float ki, float i_limit);

// regulator proporcjonalno-całkujący (PI), zastosowany w celu utrzymania temperatury na zadanym poziomie (setpoint)
// funkcja zwraca wartość sterowania `u` w zakresie [-1, 1] dla grzałki, aby zmniejszyć różnicę między wartościami `setpoint` a `measurement`
float control_pi_step(control_pi_t* pi, float setpoint, float measurement, float ts);
