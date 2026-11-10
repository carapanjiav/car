#include <stdio.h>
#include <avr/io.h>
#include "encoder.h"

float time_between_slots(){
    static float last_time = -1; 

    DDRB = 0b00000000;
    PORTB = 0b00000001;
    TCCR1A = 0b00000000;
    TCCR1B = 0b11000101;

    if (TIFR1 & (1 << ICF1)) {
        uint16_t elapsed_ticks = ICR1;
        TCNT1 = 0;
        TIFR1 = (1 << ICF1);
        last_time = elapsed_ticks * 0.064;
    }
    return last_time;
}
