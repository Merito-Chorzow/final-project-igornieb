#include <string.h>
#include <math.h>
#include "control_pi.h"

void control_pi_init(control_pi_t* pi, float kp, float ki, float integral_limit) {
    // wyzerowanie stanu, ustawienie parametrów
    pi->integral_accumulator = 0.0f;
    pi->previous_error = 0.0f;
    pi->kp = kp;
    pi->ki = ki;
    pi->integral_limit = integral_limit;
    pi->u_min = -1.0f;
    pi->u_max = 1.0f;
}

float control_pi_step(control_pi_t* pi, float setpoint, float measurement, float ts) {
    // regulator proporcjonalno-całkujący (PI), zastosowany w celu utrzymania temperatury na zadanym poziomie (setpoint)
    // funkcja zwraca wartość sterowania `u` w zakresie [-1, 1] dla grzałki, aby zmniejszyć różnicę między wartościami `setpoint` a `measurement`

    
    // obliczamy błąd przed regulacją
    float e = setpoint - measurement;
    
    // określenie składowej proporcjonalnej - im większy błąd, tym silniejsza reakcja
    // samo P (u_p) nie wystarcza do eliminacji błędu ustalonego ponieważ przy stałym błędzie całka nie narasta
    float u_p = pi->kp * e;
    
    // obliczenie składowej całkującej
    // całka narasta proporcjonalnie do błędu i czasu (ts)
    // pozwala to eliminować błąd ustalony (steady-state error)
    // dodanie czasu próbkowania (ts) zapewnia, że całka jest niezależna od częstotliwości wywołań
    pi->integral_accumulator += pi->ki * e * ts;
    
    // Anti-windup - ograniczenie całki
    // Jeśli całka rośnie bez końca (setpoint nigdy nie będzie osiągnięty),
    // może spowodować "odbicie" (bounce) i oscylacje
    if (pi->integral_accumulator > pi->integral_limit) pi->integral_accumulator = pi->integral_limit;
    if (pi->integral_accumulator < -pi->integral_limit) pi->integral_accumulator = -pi->integral_limit;
    
    // saturacja sterowania u
    // Urządzenie (grzałka, wentylator) ma ograniczenia fizyczne [-1, 1]
    // Bez saturacji moglibyśmy wysłać u=10, a system by to ignorował
    float u = u_p + pi->integral_accumulator;
    if (u > pi->u_max) u = pi->u_max;
    if (u < pi->u_min) u = pi->u_min;
    
    pi->previous_error = e;
    return u;
}
