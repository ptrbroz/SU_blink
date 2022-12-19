
#include <Arduino.h>
#include <LedDriver.h>
#include <animations.h>
#include <common.h>

animType activeAnimation = none;
uint16_t accumulatedTicks = 0; //var to hold ticks of timer A, which counts from 0 to TIMER_TOP @ 1MHz
uint16_t accumulatedMs = 0; 
uint16_t lastCounterVal = 0;

uint8_t animVars[20]; //variables to hold animation states between animate() calls.
uint8_t modState = 0; //for holding state of modification (which variable, if any, is currently being modified)

volatile uint16_t msPerFrame = 100;

const void animateNothing(int first, int mod, int delta);
const void animateRunningDot(int first, int mod, int delta);;

//array of animation functions. When 1st argument is true, this is an inicialization call. When 2nd argument is true, this is a modButton call.
//When 3rd argument is nonzero, this is a variable modification by value of 3rd argument.
const void (*ap[]) (int, int, int) = {
    animateNothing,
    animateRunningDot
};


void enterAnimation(animType anim){
    activeAnimation = anim;
    accumulatedMs = 0;
    accumulatedTicks = 0;
    (*ap[activeAnimation])(1,0,0);
}

void readTime(){
    uint16_t thisTick = TCA0_SINGLE_CNT;
    int diff = thisTick - lastCounterVal;
    if(diff > 0){
        accumulatedTicks += diff;
    }
    else{
        accumulatedTicks += (TIMER_TOP-lastCounterVal)+thisTick;
    }
    if(accumulatedTicks > 1000){
        uint16_t msToAdd = accumulatedTicks / 1000;
        accumulatedMs += msToAdd;
        accumulatedTicks -= msToAdd*1000;
    }
    lastCounterVal = thisTick;
}

void animate(){
    readTime();
    if(accumulatedMs > msPerFrame){
        accumulatedMs -= msPerFrame;
        (*ap[activeAnimation])(0,0,0);
    }
}

//Called by animate functions during modification
void pwmsOnOff(uint8_t left, uint8_t right, uint8_t top, uint8_t on){
    if(on == 0){
        PWM_CMP_LEFT = 0;
        PWM_CMP_TOP = 0;
        PWM_CMP_RIGHT = 0;
    }
    else{
        uint16_t val = TIMER_TOP / 10;
        if(left){
            PWM_CMP_LEFT = val;
        }
        if(right){
            PWM_CMP_RIGHT = val;
        }
        if(top){
            PWM_CMP_TOP = val;
        }
    }
}



const void animateNothing(int init, int mod, int delta){
    clearBuffer();
    PWM_CMP_LEFT = 0;
    PWM_CMP_TOP = 0;
    PWM_CMP_RIGHT = 0;
}

#define position animVars[0]
#define tailLen animVars[1]
#define direction animVars[2]
#define brightness animVars[3]
const void animateRunningDot(int init, int mod, int delta){

    if(init){
        msPerFrame = 2000;
        position = 17;
        tailLen = 0;
        direction = 1;
        modState = 0;
        return;
    }

    int spos = position;
    int sdir = 1;
    if(!direction){
        sdir = -1;
    }

    clearBuffer();

    //draw dot and tail
    for(int i = 0; i < 1 + tailLen; i++){

        spos += sdir;

        if(spos >= CIRCLE_LED_COUNT){
            spos = 0;
        }
        else if(spos < 0){
            spos = CIRCLE_LED_COUNT - 1;
        }

        circleEnableBuffer[CIRCLE_PWM_DEPTH*position] = portEnableSequence[position];

        if(i==0){
            position = (uint8_t) spos; //save position for next call
            sdir = -sdir; //draw tail in reverse direction of movement
        }
    }


    uint16_t pwmFraction = ((uint8_t)(TIMER_TOP/20)) * ((position%6) + 1 );

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
#undef position








