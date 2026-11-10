#include "speed.h"
#include <stddef.h>

#define PI 3.14f

int calculate_speed(
    int slots,
    float wheel_radius,
    float time_between_slots,
    float gear_ratio,
    float *rpm,
    float *speed
) {
    if (slots <= 0 ||
        wheel_radius <= 0 ||
        time_between_slots <= 0 ||
        gear_ratio <= 0 ||
        rpm == NULL ||
        speed == NULL) {
        return -1;
}

    float encoder_rpm = 60.0f / (slots * time_between_slots);

    *rpm = encoder_rpm / gear_ratio;

    *speed = (2.0f * PI * wheel_radius * *rpm) / 60.0f;

    return 0;
}