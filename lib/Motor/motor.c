#include <avr/io.h>
#include <stdint.h>
#include "motor.h"

// Initialize motor control
void motor_init(void)
{
    // Prepare D6 with a LOW output level
    PORTD &= ~(1 << PD6);

    // Configure D6 as an output
    DDRD |= (1 << DDD6);

    // Configure Timer0 for Fast PWM mode
    // Keep the PWM output disconnected for now
    TCCR0A = (1 << WGM01) | (1 << WGM00);

    // Reset the timer counter and duty-cycle register
    TCNT0 = 0;
    OCR0A = 0;

    // Set the timer clock to CPU clock / 1024
    TCCR0B = (1 << CS02) | (1 << CS00);
}

// Disable the motor drive signal
void motor_stop(void)
{
    // Disconnect D6 from the timer's PWM output
    TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));

    // Hold D6 LOW
    PORTD &= ~(1 << PD6);
}

// Set the duty cycle from 0% to 100%
void motor_set_duty(unsigned char percent)
{
    if (percent == 0)
    {
        motor_stop();
    }
    else if (percent >= 100)
    {
        // For 100% duty, hold D6 HIGH instead of using PWM
        TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));
        PORTD |= (1 << PD6);
    }
    else
    {
        // Convert the percentage to a timer compare value
        OCR0A = (uint8_t)(
            ((uint32_t)percent * 256UL + 50UL) / 100UL - 1UL
        );

        // Enable non-inverting PWM output on D6
        TCCR0A &= ~(1 << COM0A0);
        TCCR0A |= (1 << COM0A1);
    }
}