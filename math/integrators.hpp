#pragma once

struct ExplicitEuler {
    template<typename System, typename State>
    State step(System&& sys, double t, State y, double dt) {
        return y + dt * sys(t,y);
    }
};

struct SemiImplicitEuler {
    template<typename AccelFn, typename State>
    State step(AccelFn&& accel, State y, double dt) {

        double a = accel(y.x);
        y.v += dt * a;
        y.x += dt * y.v;

        return y;
    }
};

struct RK4 {
    template<typename System, typename State>
    State step(System&& sys, double t, State y, double dt) {
        State k1 = sys(t,y);
        State k2 = sys(t + dt/2, y + dt/2 * k1);
        State k3 = sys(t + dt/2, y + dt/2 *  k2);
        State k4 = sys(t + dt, y + dt * k3);
 
        return y + (dt/6.0) * (k1 + 2 * k2 + 2 * k3 + k4);
    }
};