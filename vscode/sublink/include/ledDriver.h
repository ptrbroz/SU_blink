#ifndef LEDDRIVER_H
#define LEDDRIVER_H


#include <Arduino.h>

#define CIRCLE_LED_COUNT 18
#define CIRCLE_PWM_DEPTH  5

#define L0 4
#define L1 5
#define L2 3
#define L3 6
#define L4 7

#define PWM_CMP_TOP     TCA0_SINGLE_CMP0
#define PWM_CMP_LEFT    TCA0_SINGLE_CMP1
#define PWM_CMP_RIGHT   TCA0_SINGLE_CMP2

extern const uint8_t portOutSequence[CIRCLE_LED_COUNT];
extern const uint8_t portEnableSequence[CIRCLE_LED_COUNT];
extern uint8_t circleEnableBuffer[CIRCLE_LED_COUNT*CIRCLE_PWM_DEPTH];

//todo pwm levels?

void clearBuffer();


#endif