#include <string.h>
#include <math.h>
#include "control_pi.h"

void control_pi_init(control_pi_t* pi, float kp, float ki, float integral_limit) {
    pi->integral_accumulator = 0.0f;
    pi->kp = kp;
    pi->ki = ki;
    pi->integral_limit = integral_limit;
    pi->u_min = -1.0f;
    pi->u_max = 1.0f;
}

float control_pi_step(control_pi_t* pi, float setpoint, float measurement, float ts) {
    // regulator proporcjonalno-całkujący (PI), zastosowany w celu utrzymania temperatury na zadanym poziomie (setpoint)
    // funkcja zwraca wartość sterowania w zakresie [u_min, u_max] dla grzałki, aby zmniejszyć różnicę między wartościami `setpoint` a `measurement`
    
    // obliczamy błąd wzgledny między wartością zadaną a zmierzoną
    float e = setpoint - measurement;
    
    // określenie składowej proporcjonalnej - wpływa ona na reakcję systemu na bieżący błąd
    // im większy błąd, tym silniejsza reakcja
    // samo P (u_p) nie wystarczyłoby do eliminacji błędu ustalonego ponieważ przy stałym błędzie całka nie będzie rosła
    float u_p = pi->kp * e;
    
    // obliczenie składowej całkującej
    // całka narasta proporcjonalnie do błędu i czasu (ts)
    // dodanie czasu próbkowania (ts) zapewnia, że całka jest niezależna od częstotliwości wywołań
    pi->integral_accumulator += pi->ki * e * ts;
    
    // Anti-windup - ograniczenie całki
    if (pi->integral_accumulator > pi->integral_limit) pi->integral_accumulator = pi->integral_limit;
    if (pi->integral_accumulator < -pi->integral_limit) pi->integral_accumulator = -pi->integral_limit;
    
    // Ograniczenie reakcji do zakresu fizycznego aktuatora
    // Wartość sterowania `u` musi mieścić się w zakresie [u_min, u_max]
    float u = u_p + pi->integral_accumulator;
    if (u > pi->u_max) u = pi->u_max;
    if (u < pi->u_min) u = pi->u_min;
    
    return u;
}
