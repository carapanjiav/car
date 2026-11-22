/*
#ifndef ENCODER_H
#define ENCODER_H

// Returns the time between slots in milliseconds.

float time_between_slots();

#endif
*/



#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

void encoder_init(void);
void encoder_start(void);
void encoder_stop(void);

uint8_t encoder_get_period_ms(float *period_ms);

#endif
