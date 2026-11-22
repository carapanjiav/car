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
#include "motor.h"

#define EV_START_BUTTON  0xA1
#define EV_DUTY_SLIDER   0xA2

#define ENCODER_SLOTS       8
#define WHEEL_RADIUS_MM     32.0f
#define GEAR_RATIO          1.0f
#define DISPLAY_UPDATE_MS   100U

#define PAGE_START  "pageStart"
#define PAGE_SPEED  "pageSpeed"
#define PAGE_INFO   "pageInfo"

int main(void)
{
    uart_init();
    io_redirect();
    encoder_init();
    motor_init();

    float rpm = 0.0f;
    float speed_mm_s = 0.0f;
    float period_ms = 0.0f;
    uint8_t event_id = 0;
    uint8_t event_value = 0;
    uint8_t running = 0;
    uint8_t duty = 0; 
    uint16_t elapsed_ms = 0;

    nex_cmd(PAGE_SPEED ".t0.txt=\"Waiting for encoder...\"");
    nex_cmd(PAGE_SPEED ".t1.txt=\"RPM: --\"");
    nex_cmd(PAGE_SPEED ".t2.txt=\"Speed: --\"");
    nex_cmd(PAGE_SPEED ".t3.txt=\"PAUSED\"");
    nex_val(PAGE_START ".h0", duty);
    nex_cmd(PAGE_START ".t0.txt=\"Speed: %u%%\"", duty);

    while (1) {
        while (nex_event(&event_id, &event_value)) {
            switch (event_id) {
                case EV_START_BUTTON:
                    if (running) {
                        running = 0;
                        encoder_stop();
                        motor_stop();
                        nex_cmd(PAGE_START ".b0.txt=\"Start\"");
                        nex_cmd(PAGE_SPEED ".t3.txt=\"PAUSED\"");
                    } else {
                        running = 1;
                        encoder_start();
                        motor_set_duty(duty);
                        rpm = 0.0f;
                        speed_mm_s = 0.0f;
                        period_ms = 0.0f;
                        nex_cmd(PAGE_START ".b0.txt=\"Stop\"");
                        nex_cmd(PAGE_SPEED ".t0.txt=\"Waiting for encoder...\"");
                        nex_cmd(PAGE_SPEED ".t1.txt=\"RPM: --\"");
                        nex_cmd(PAGE_SPEED ".t2.txt=\"Speed: --\"");
                        nex_cmd(PAGE_SPEED ".t3.txt=\"RUNNING\"");
                    }
                    break;
                    
                case EV_DUTY_SLIDER:
                    duty = (event_value > 100U) ? 100U : event_value;
                    nex_cmd(PAGE_START ".t0.txt=\"Speed: %u%%\"", duty);
                    if (running) {
                        motor_set_duty(duty);
                    }
                    break;

                default:
                    break;
            }
        }

        if (elapsed_ms == 0U) {
            if (running) {
                if (encoder_get_period_ms(&period_ms)) {
                    int result = calculate_speed(
                        ENCODER_SLOTS,
                        WHEEL_RADIUS_MM,
                        period_ms / 1000.0f,
                        GEAR_RATIO,
                        &rpm,
                        &speed_mm_s
                    );

                    if (result == 0) {
                        nex_cmd(PAGE_SPEED ".t0.txt=\"Period: %.2f ms\"", period_ms);
                        nex_cmd(PAGE_SPEED ".t1.txt=\"RPM: %.2f\"", rpm);
                        nex_cmd(PAGE_SPEED ".t2.txt=\"Speed: %.2f mm/s\"", speed_mm_s);
                    } else {
                        nex_cmd(PAGE_SPEED ".t0.txt=\"Waiting for encoder...\"");
                        nex_cmd(PAGE_SPEED ".t1.txt=\"RPM: --\"");
                        nex_cmd(PAGE_SPEED ".t2.txt=\"Speed: --\"");
                    }
                } else {
                    rpm = 0.0f;
                    speed_mm_s = 0.0f;
                    nex_cmd(PAGE_SPEED ".t0.txt=\"Waiting for encoder...\"");
                    nex_cmd(PAGE_SPEED ".t1.txt=\"RPM: 0.00\"");
                    nex_cmd(PAGE_SPEED ".t2.txt=\"Speed: 0.00 mm/s\"");
                }
            }
            
            nex_cmd(PAGE_INFO ".t0.txt=\"Voltage: %.2f V\"", voltage());
        }

        _delay_ms(10);
        elapsed_ms += 10U;
        if (elapsed_ms >= DISPLAY_UPDATE_MS) {
            elapsed_ms = 0U;
        }
    }
}