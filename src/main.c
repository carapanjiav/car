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
#include "voltage.h"

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
    uint8_t running = 0;
    uint16_t update_interval_ms = 500;
    uint16_t elapsed_ms = 0;

    while (1) {
        while (nex_event(&event_id, &event_value)) {
            switch (event_id) {
                case EV_START_BUTTON:
                    running = !running;

                    if(running){nex_cmd("page0.b0.txt=\"Stop\"");} 
                    else {nex_cmd("page0.b0.txt=\"Start\"");}

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
                nex_cmd("pageSpeed.t0.txt=\"Time: %.2f\"", time);
                nex_cmd("pageSpeed.t1.txt=\"RPM: %.2f\"", rpm);
                nex_cmd("pageSpeed.t2.txt=\"Speed: %.2f mm/s\"", speed);
            } else {
                nex_cmd("pageSpeed.t0.txt=\"Waiting for encoder...\"");
                nex_cmd("pageSpeed.t1.txt=\"RPM: --\"");
                nex_cmd("pageSpeed.t2.txt=\"Speed: --\"");
            }

                nex_cmd("pageInfo.t0.txt=\"Voltage: %.2f V\"", voltage);            
        }

        _delay_ms(10);
        elapsed_ms += 10U;
        if (elapsed_ms >= update_interval_ms) elapsed_ms = 0U;
    }
}
