#ifndef SPEED_H
#define SPEED_H

// Calculates wheel RPM and speed.
// Returns 0 on success, -1 if the input is invalid.

int calculate_speed(
    int slots,
    float wheel_radius,
    float time_between_slots,
    float gear_ratio,
    float *rpm,
    float *speed
);

#endif