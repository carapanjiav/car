#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdio.h>
#include "usart.h"
#include "speed.h"
#include "encoder.h"

#define EV_START_BUTTON  0xA1

const int holes = 8;
const float wheel_radius = 32;
const float gear_ratio = 1;

int main(void)
{
    uart_init();
    io_redirect();

    float rpm = 0;
    float speed = 0;
    uint8_t event_id;
    uint8_t event_value;
    uint8_t running = 1;
    uint16_t update_interval_ms = 500;
    uint16_t elapsed_ms = 0;

    while (1) {
        while (nex_event(&event_id, &event_value)) {
            switch (event_id) {
                case EV_START_BUTTON:
                    running = !running;
                    break;

                default:
                    break;
            }
        }

        if (running && elapsed_ms == 0U) {
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
            } else {
                t0_text("Waiting for encoder...");
                t1_text("RPM: --");
                t2_text("Speed: --");
            }
        }

        _delay_ms(10);
        elapsed_ms += 10U;
        if (elapsed_ms >= update_interval_ms) elapsed_ms = 0U;
    }
}
