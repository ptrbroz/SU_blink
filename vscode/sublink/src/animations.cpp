
#include <Arduino.h>
#include <LedDriver.h>
#include <animations.h>

uint8_t position = 0;

animType activeAnimation = none;
uint16_t accumulatedTime = 0;
uint8_t lastCounterVal = 0;

const void animateNothing();
const void animateRunningDot();

const uint16_t animTimes[] = {
    1000,   //none
    1000    //runningDot
    };

const void (*ap[]) () = {
    animateNothing,
    animateRunningDot
};


void enterAnimation(animType anim){
    activeAnimation = anim;
    accumulatedTime = 0;
    switch(anim){
        case runningDot:
            position = 0;
            break;
        default:
            break;
    }
}

void readTime(){
    uint8_t thisTime = (uint8_t) TCA0_SINGLE_CNT;
    int16_t diff = thisTime - lastCounterVal;
    if(diff > 0){
        accumulatedTime += diff;
    }
    else{
        accumulatedTime += (255-lastCounterVal)+thisTime;
    }
    lastCounterVal = thisTime;
}

void animate(){
    readTime();
    //todo check specific
    if(accumulatedTime > animTimes[activeAnimation]){
        accumulatedTime = 0;
        (*ap[activeAnimation])();
    }
}



const void animateNothing(){
    PWM_CMP_LEFT = 5;
    circleEnableBuffer[CIRCLE_PWM_DEPTH*11] = portEnableSequence[11];
}

const void animateRunningDot(){

    position += 1;
    if(position >= CIRCLE_LED_COUNT){
        position = 0;
    }
    clearBuffer();

    circleEnableBuffer[CIRCLE_PWM_DEPTH*position]   = portEnableSequence[position];
    circleEnableBuffer[CIRCLE_PWM_DEPTH*position+1] = portEnableSequence[position];
    circleEnableBuffer[CIRCLE_PWM_DEPTH*position+2] = portEnableSequence[position];

    uint8_t pwmFraction = ((uint8_t)(30.0/6.0)) * ((position%6) + 1);

    
    if(position < 6){
        PWM_CMP_LEFT = pwmFraction;
        PWM_CMP_TOP = 0;
        PWM_CMP_RIGHT = 0;
    }
    else if(position < 12){
        PWM_CMP_RIGHT = 0;
        PWM_CMP_TOP = pwmFraction;
        PWM_CMP_LEFT = 0;
    }
    else{
        PWM_CMP_LEFT = 0;
        PWM_CMP_TOP = 0;
        PWM_CMP_RIGHT = pwmFraction;
    }
    
}










