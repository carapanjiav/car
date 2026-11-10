#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "usart.h"
#include "speed.h"
#include "encoder.h"

const int holes = 8;
const float wheel_radius = 32;
const float gear_ratio = 1;

int main(void) //asd
{
    uart_init();
    io_redirect();

    float rpm = 0;
    float speed = 0;

    while (1) {
        float time = time_between_slots() / 1000.0f;

        int result = calculate_speed(
            holes,
            wheel_radius,
            time,
            gear_ratio,
            &rpm,
            &speed
        );

        if (result == 0) {
            t0_text("Time: %.2f s", time);
            t1_text("RPM: %.2f", rpm);
            t2_text("Speed: %.2f mm/s", speed);
        }
        else {
            t0_text("Error calculating wheel speed.");
            t1_text("");
            t2_text("");
        }
        _delay_ms(1000);
    }

    return 0;
}
