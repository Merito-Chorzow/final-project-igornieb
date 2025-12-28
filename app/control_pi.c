#include <string.h>
#include <math.h>
#include "control_pi.h"

void control_pi_init(control_pi_t* pi, float kp, float ki, float i_limit) {
    // wyzerowanie stanu, ustawienie parametrów
    pi->i_acc = 0.0f;
    pi->e_prev = 0.0f;
    pi->kp = kp;
    pi->ki = ki;
    pi->i_limit = i_limit;
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
    pi->i_acc += pi->ki * e * ts;
    
    // Anti-windup - ograniczenie całki
    // Jeśli całka rośnie bez końca (setpoint nigdy nie będzie osiągnięty),
    // może spowodować "odbicie" (bounce) i oscylacje
    if (pi->i_acc > pi->i_limit) pi->i_acc = pi->i_limit;
    if (pi->i_acc < -pi->i_limit) pi->i_acc = -pi->i_limit;
    
    // saturacja sterowania u
    // Urządzenie (grzałka, wentylator) ma ograniczenia fizyczne [-1, 1]
    // Bez saturacji moglibyśmy wysłać u=10, a system by to ignorował
    float u = u_p + pi->i_acc;
    if (u > pi->u_max) u = pi->u_max;
    if (u < pi->u_min) u = pi->u_min;
    
    pi->e_prev = e;
    return u;
}
