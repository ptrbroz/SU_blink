
#include <Arduino.h>
#include <LedDriver.h>
#include <animations.h>
#include <common.h>

animType activeAnimation = none;
uint16_t accumulatedTicks = 0; //var to hold ticks of timer A, which counts from 0 to TIMER_TOP @ 1MHz
uint16_t accumulatedMs = 0; 
uint16_t lastCounterVal = 0;

uint8_t animVars[20]; //variables to hold animation states between animate() calls.
uint8_t modState = 0; //for holding state of modification (which variable, if any, is currently being modified). 0 reserved for "not currently modifying anything" (-> can change anim)

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

void handleModButton(){
    (*ap[activeAnimation])(0,1,0);
}

void handleModify(int value){
    if(!modState){
        //change active animation
        //todo
    }
    else{
        //allow active animation to modify its variable
        (*ap[activeAnimation])(0,0,value);
    }
}

//Called by animate functions during modification
void pwmsOnOff(uint8_t left, uint8_t right, uint8_t top){
    static uint8_t on = 1;
    on = !on;
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
#define brightness animVars[2]
#define direction animVars[3]
const void animateRunningDot(int init, int mod, int delta){

    if(init){
        msPerFrame = 100;
        position = 0;
        tailLen = 3;
        direction = 1;
        brightness = 5;
        modState = 0;
        return;
    }

    if(mod){
        if(modState >= 4){
            modState = 0;
        }
        else{
            modState++;
        }
        return;
    }

    if(modState && delta != 0){
        if(modState == 4){
            int newMs = msPerFrame + 10*delta;
            if(newMs < 0){
                msPerFrame = 1;
            }
            else{
                msPerFrame = newMs;
            }
        }
        else{
            int newVal = animVars[modState] + delta;
            if(newVal < 0) newVal = 0;
            if(modState == 1 && newVal > CIRCLE_LED_COUNT - 1) newVal = CIRCLE_LED_COUNT - 1;
            if(modState == 2 && newVal > CIRCLE_PWM_DEPTH) newVal = CIRCLE_PWM_DEPTH;
            if(modState == 3 && newVal > 1) newVal = 1;
            animVars[modState] = newVal;
        }
    }

    int spos = position;
    int sdir = 1;
    if(!direction){
        sdir = -1;
    }

    clearBuffer();

    uint8_t bArr[CIRCLE_LED_COUNT];
    bArr[0] = brightness;
    uint8_t stepsPerDrop = tailLen / (brightness - 1);

    for(uint8_t i = 0; i < tailLen; i++){
        int b = bArr[0] - 1 - (i/stepsPerDrop);
        if(b<=0){
            bArr[1+i] = 1;
        }
        else{
            bArr[1+i] = (uint8_t) b;
        }
    }

    //draw dot and tail
    for(uint8_t i  = 0; i < 1 + tailLen; i++){

        spos += sdir;

        if(spos >= CIRCLE_LED_COUNT){
            spos = 0;
        }
        else if(spos < 0){
            spos = CIRCLE_LED_COUNT - 1;
        }

        for(int j = 0; j < bArr[i]; j++){
            circleEnableBuffer[CIRCLE_PWM_DEPTH*(uint8_t)spos + j] = portEnableSequence[(uint8_t) spos];
        }

        if(i==0){
            position = (uint8_t) spos; //save position for next call
            sdir = -sdir; //draw tail in reverse direction of movement
        }
    }

    if(modState){
        //flash pwm led if in modstate
        pwmsOnOff((modState == 1 | modState == 4), (modState == 2 | modState == 4 ), (modState == 3 | modState == 4));
    }
    else{
        //show pwm depending on position of dot
        uint8_t oldPos = position;
        uint16_t pwmFraction = ((uint8_t)(TIMER_TOP/60)) * ((oldPos%6) + 1 );

        if(oldPos < 6){
            PWM_CMP_LEFT = pwmFraction;
            PWM_CMP_TOP = 0;
            PWM_CMP_RIGHT = 0;
        }
        else if(oldPos < 12){
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
}
#undef tailLen 
#undef direction
#undef brightness
#undef position








