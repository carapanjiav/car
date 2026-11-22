#ifndef MOTOR_H
#define MOTOR_H

void motor_init(void);
void motor_set_duty(unsigned char percent);
void motor_stop(void);

#endif