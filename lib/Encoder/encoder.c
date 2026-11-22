/*
#include <stdio.h>
#include <avr/io.h>
#include "encoder.h"

float time_between_slots(){
    static float last_time = -1; 

    DDRB = 0b00000000;
    PORTB = 0b00000001;
    TCCR1A = 0b00000000;
    TCCR1B = 0b10000101;

    if (TIFR1 & (1 << ICF1)) {
        uint16_t elapsed_ticks = ICR1;
        TCNT1 = 0;
        TIFR1 = (1 << ICF1);
        last_time = elapsed_ticks * 0.064;
    }
    return last_time;
}
*/


    #ifndef F_CPU
    #define F_CPU 16000000UL
    #endif

    #include <avr/io.h>
    #include <avr/interrupt.h>
    #include <stdint.h>
    #include "encoder.h"

    #ifndef ENCODER_TIMEOUT_MS
    #define ENCODER_TIMEOUT_MS 2000UL
    #endif

    #define TIMER1_PRESCALER       1024UL
    #define TIMER1_TICK_US         ((TIMER1_PRESCALER * 1000000UL) / F_CPU)
    #define ENCODER_TIMEOUT_TICKS  ((ENCODER_TIMEOUT_MS * 1000UL) / TIMER1_TICK_US)
    #define TIMER1_CLOCK_BITS      (_BV(CS12) | _BV(CS10))

    #define ENCODER_MIN_PERIOD_MS 2UL

    #define ENCODER_MIN_PERIOD_TICKS \
        ((ENCODER_MIN_PERIOD_MS * 1000UL) / TIMER1_TICK_US)

    static volatile uint32_t timer1_overflows = 0;
    static volatile uint32_t previous_capture = 0;
    static volatile uint32_t last_pulse_timestamp = 0;
    static volatile uint32_t latest_period_ticks = 0;
    static volatile uint8_t have_previous_capture = 0;
    static volatile uint8_t have_valid_period = 0;
    static volatile uint8_t encoder_running = 0;

    ISR(TIMER1_OVF_vect)
    {
        timer1_overflows++;
    }

    ISR(TIMER1_CAPT_vect)
    {
        uint16_t captured = ICR1;
        uint32_t high = timer1_overflows;

        if ((TIFR1 & _BV(TOV1)) && captured < 0x8000U) {
            high++;
        }

        uint32_t timestamp = (high << 16) | captured;

        if (have_previous_capture) {

            uint32_t period = timestamp - previous_capture;

            if (period >= ENCODER_MIN_PERIOD_TICKS) {

                latest_period_ticks = period;
                have_valid_period = 1;

                previous_capture = timestamp;
                last_pulse_timestamp = timestamp;
            }

        } else {

            previous_capture = timestamp;
            last_pulse_timestamp = timestamp;
            have_previous_capture = 1;
        }
    }

    void encoder_init(void)
    {
        uint8_t old_sreg = SREG;
        cli();
        DDRB &= (uint8_t)~_BV(DDB0);
        PORTB |= _BV(PORTB0);

        TCCR1A = 0;
        TCCR1B = 0;                 
        TCNT1 = 0;
        TIMSK1 = 0;
        TIFR1 = _BV(ICF1) | _BV(TOV1);

        timer1_overflows = 0;
        previous_capture = 0;
        last_pulse_timestamp = 0;
        latest_period_ticks = 0;
        have_previous_capture = 0;
        have_valid_period = 0;
        encoder_running = 0;

        SREG = old_sreg;
    }

    void encoder_start(void)
    {
        uint8_t old_sreg = SREG;
        cli();

        TCCR1B = 0;
        TCNT1 = 0;
        timer1_overflows = 0;
        previous_capture = 0;
        last_pulse_timestamp = 0;
        latest_period_ticks = 0;
        have_previous_capture = 0;
        have_valid_period = 0;
        TIFR1 = _BV(ICF1) | _BV(TOV1);

        TIMSK1 = _BV(ICIE1) | _BV(TOIE1);
        encoder_running = 1;
        TCCR1B = _BV(ICNC1) | _BV(ICES1) | TIMER1_CLOCK_BITS;

        SREG = old_sreg;
    }

    void encoder_stop(void)
    {
        uint8_t old_sreg = SREG;
        cli();

        TCCR1B = 0;
        TIMSK1 = 0;
        encoder_running = 0;

        SREG = old_sreg;
    }

    uint8_t encoder_get_period_ms(float *period_ms)
    {
        if (period_ms == 0) {
            return 0;
        }

        uint8_t old_sreg = SREG;
        cli();

        uint8_t running = encoder_running;
        uint8_t valid = have_valid_period;
        uint32_t period = latest_period_ticks;
        uint32_t last_pulse = last_pulse_timestamp;

        uint32_t high = timer1_overflows;
        uint16_t low = TCNT1;
        if (TIFR1 & _BV(TOV1)) {
            high++;
            low = TCNT1;
        }
        uint32_t now = (high << 16) | low;

        SREG = old_sreg;

        if (!running || !valid || (uint32_t)(now - last_pulse) > ENCODER_TIMEOUT_TICKS) {
            *period_ms = 0.0f;
            return 0;
        }

        *period_ms = (float)period * ((float)TIMER1_TICK_US / 1000.0f);
        return 1;
    }
