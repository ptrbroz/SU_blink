
#include <Arduino.h>
#include "ledDriver.h"

#define LOW 0
#define HIGH 1
#define Z 2

const uint8_t portOutSequence[CIRCLE_LED_COUNT] = {
    1 << L4, //first LED = D16
    1 << L3,
    1 << L4,
    1 << L2,
    1 << L4,
    1 << L1,
    1 << L0, //7th led = D4
    1 << L3,
    1 << L1,
    1 << L0,
    1 << L2,
    1 << L0,
    1 << L2, //13th led = D10
    1 << L1,
    1 << L3,
    1 << L1,
    1 << L3,
    1 << L2};

const uint8_t portEnableSequence[CIRCLE_LED_COUNT] = {
    1 << L4 | 1 << L3, //first LED = D16
    1 << L3 | 1 << L4,
    1 << L4 | 1 << L2,
    1 << L2 | 1 << L4,
    1 << L4 | 1 << L1,
    1 << L1 | 1 << L4,
    1 << L0 | 1 << L3, //7th led = D4
    1 << L3 | 1 << L0,
    1 << L1 | 1 << L0,
    1 << L0 | 1 << L1,
    1 << L2 | 1 << L0,
    1 << L0 | 1 << L2,
    1 << L2 | 1 << L1, //13th led = D10
    1 << L1 | 1 << L2,
    1 << L3 | 1 << L1,
    1 << L1 | 1 << L3,
    1 << L3 | 1 << L2,
    1 << L2 | 1 << L3};
    
uint8_t circleEnableBuffer[CIRCLE_LED_COUNT*CIRCLE_PWM_DEPTH];

void clearBuffer(){
    memset(circleEnableBuffer, 0, sizeof(circleEnableBuffer));
}
