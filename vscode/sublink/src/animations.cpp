
#include <Arduino.h>
#include <LedDriver.h>
#include <animations.h>
#include <common.h>

uint8_t activeAnimation = 0;
uint16_t accumulatedTicks = 0; //var to hold ticks of timer A, which counts from 0 to TIMER_TOP @ 1MHz
uint16_t accumulatedMs = 0; 
uint16_t lastCounterVal = 0;

uint8_t animVars[20]; //variables to hold animation states between animate() calls.
const uint8_t maxModifiableVars = 5; //first maxModifiableVars may be modified by user via buttons
uint8_t modState = 0; //for holding state of modification (which variable, if any, is currently being modified). 0 reserved for "not currently modifying anything" (-> can change anim)
const uint8_t modStateMs = 0xfe; //when modstate is this, the msPerFrame value is being modified
uint8_t modifiableVars = 0; //set by animation function to signify how many variables can be modified (starting at index 0).
uint8_t animVarsMaximums[maxModifiableVars]; //this holds maximum values attainable by modifiable variables. Set by anim functions on first call
uint8_t animVarsMinimums[maxModifiableVars]; //this holds minimum values attainable by modifiable variables. Set by anim functions on first call



uint8_t msPerFrameModifiable = 1; //may be set to 0 by animation function to disallow modification of msPerFrame
volatile uint16_t msPerFrame = 100;

const void animateRunningDot(int first);
const void animatePendulumClock(int firts);
const void animatePulsingLight(int first);
const void animateStackingDots(int first);
const void animateMarkerEraser(int first);

//array of animation functions. When 1st argument is true, this is an inicialization call. When 2nd argument is true, this is a modButton call.
//When 3rd argument is nonzero, this is a variable modification by value of 3rd argument.
const void (*ap[]) (int) = {
    animateRunningDot,
    animatePendulumClock,
    animatePulsingLight,
    animateStackingDots,
    animateMarkerEraser
};


void enterAnimation(uint8_t anim){
    activeAnimation = anim;
    accumulatedMs = 0;
    accumulatedTicks = 0;
    memset(animVarsMaximums, 1, sizeof(animVarsMaximums));
    memset(animVarsMinimums, 0, sizeof(animVarsMinimums));
    msPerFrameModifiable = 1;
    (*ap[activeAnimation])(1);
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

//Called during modification
void pwmsOnOff(uint8_t left, uint8_t right, uint8_t top){
    static uint8_t on = 1;
    static uint16_t prescaleCounter = 0;
    uint16_t target = 100/msPerFrame;
    prescaleCounter++;
    if(prescaleCounter > target){
        prescaleCounter = 0;
    }
    else{
        return;
    }
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

void animate(){
    readTime();
    if(accumulatedMs > msPerFrame){
        accumulatedMs -= msPerFrame;
        (*ap[activeAnimation])(0);
        if(modState == modStateMs){
            pwmsOnOff(1,1,1);
        }
        else if(modState){
            uint8_t mod = modState % 3;
            pwmsOnOff( (mod == 1) , (mod == 2), (mod == 0) || modState > 3);
        }
    }
}

void handleModButton(){
    modState++;
    if(modState > modifiableVars){
        if(msPerFrameModifiable && modState < modStateMs){
            modState = modStateMs;
        }
        else{
            modState = 0;
            PWM_CMP_LEFT = 0;
            PWM_CMP_TOP = 0;
            PWM_CMP_RIGHT = 0;
        }
    }
}

void handleModify(int value){
    if(!modState){
        const uint8_t animCount = sizeof(ap)/sizeof(ap[0]);
        if(value > 0){
            activeAnimation++;
            if(activeAnimation >= animCount){
                activeAnimation = 0;
            }
        }
        else{
            if(activeAnimation == 0){
                activeAnimation = animCount - 1;
            }
            else{
                activeAnimation -= 1;
            }
        }
        enterAnimation(activeAnimation);
    }
    else{
        if(modState == modStateMs){
            int newMs = msPerFrame - value*5;//invert here -- make it feel like controlling speed, not delay
            if(newMs< 10){
                msPerFrame = 10;
            }
            else{
                msPerFrame = (uint8_t) newMs;
            }
        }
        else if(modState > 0){
            uint8_t i = modState - 1;
            int newVal = animVars[i] + value;
            if(newVal <= animVarsMaximums[i] && newVal >= animVarsMinimums[i]){
                animVars[i] = newVal;
            }
        }
    }
}




#define tailLen animVars[0]
#define brightness animVars[1]
#define direction animVars[2]
#define position animVars[3]
const void animateRunningDot(int init){

    if(init){
        msPerFrame = 100;
        position = 0;
        tailLen = 3;
        direction = 1;
        brightness = 5;
        modifiableVars = 3;
        animVarsMaximums[0] = CIRCLE_LED_COUNT - 1;
        animVarsMinimums[1] = 1;
        animVarsMaximums[1] = CIRCLE_PWM_DEPTH;
        return;
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

    if(!modState){
        //show pwm depending on position of dot
        uint8_t oldPos = position;
        uint16_t pwmFraction = ((uint8_t)(TIMER_TOP/(18*6))) * ((oldPos%6) + 1 );

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




#define slowPendulum animVars[0]
#define clockFaceMultiples animVars[1]
#define quarterSeconds animVars[2]
#define direction animVars[3]
#define index animVars[4]
#define clockFaceSeconds animVars[5]
#define slowCounter animVars[6]
const void animatePendulumClock(int first){
    //quantized LUT from pi/2 down to 0, covering 1/4 second
    const uint8_t cosLUT[26] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 15, 16, 18, 20, 21, 23, 24, 26};
    const uint8_t lutMax = sizeof(cosLUT) - 1;

    if(first){
        msPerFrame = 10;
        index = 0;
        direction = 1;
        quarterSeconds = 0;
        clockFaceSeconds = 0;
        clockFaceMultiples = 0;
        slowPendulum = 1;
        modifiableVars = 1;
        msPerFrameModifiable = 0;
        return;
    }

    if(slowPendulum){
        if(!slowCounter){
            slowCounter++;
            return;
        }
        else{
            slowCounter = 0;
        }
    }


    //if control reached here, 10ms has elapsed since last time
    int sd = (direction) ? 1 : -1;
    int nextIndex = index + sd;
    uint8_t incrementSeconds = 0;

    if(nextIndex == 0){
        //PORTB_OUT |= 0x01;
        direction = !direction;
        quarterSeconds++;
        if(quarterSeconds >= 4){
            quarterSeconds = 0;
            //dont increment seconds here, rather do it on edge between 0 and 1 qs -- at end of downswing
        }
    }
    else if(nextIndex == lutMax){
        //PORTB_OUT |= 0x01;
        direction = !direction;
        quarterSeconds++;
        if(quarterSeconds == 1 || (slowPendulum)){
            incrementSeconds = 1;
        }
    }

    if(incrementSeconds){
        if(clockFaceSeconds == 0xff){
            clockFaceSeconds = 0;
            if(clockFaceMultiples == 3){
                clockFaceMultiples = 0;
            }
            else{
                clockFaceMultiples++;
            }
        }
        else{
            clockFaceSeconds++;
        }
    }

    index = (uint8_t) nextIndex;

    uint8_t position = cosLUT[index];
    uint8_t m = position%6;
    uint8_t sourceIntensity;
    uint8_t sinkIntensity;

    if(direction == 1){
        sourceIntensity = 6 - m;
        sinkIntensity = m;
    }
    else if(direction == 0){
        sourceIntensity = m;
        sinkIntensity = 6 - m;
    }

    //determine source, sink led positions
    const uint8_t pStartIndex = 4;
    const uint8_t pEndIndex = 13;

    uint8_t sourceIndex;
    uint8_t sinkIndex;

    if(quarterSeconds == 0){
        //downswing from 4 to 0
        sourceIndex = pStartIndex - (position / 6);
        if(sourceIndex == 0){
            sinkIndex = CIRCLE_LED_COUNT-1;
        }
        else{
            sinkIndex = sourceIndex - 1;
        }
    }
    else if(quarterSeconds == 1){
        //upswing from 17 to 13
        sourceIndex = pEndIndex + (position / 6) + 1;
        if(sourceIndex == CIRCLE_LED_COUNT){
            sourceIndex = 0;
            sinkIndex = CIRCLE_LED_COUNT - 1;
        }
        else{
            sinkIndex = sourceIndex - 1;
        }
    }
    else if(quarterSeconds == 2){
        //downswing from 13 to 17
        sourceIndex = (CIRCLE_LED_COUNT - 1) + (position/6 - 4);
        if(sourceIndex == CIRCLE_LED_COUNT - 1){
            sinkIndex = 0;
        }
        else{
            sinkIndex = sourceIndex + 1;
        }
    }
    else if(quarterSeconds == 3){
        //upswing from 0 to 4
        sinkIndex = pStartIndex - position/6;
        if(sinkIndex == 0){
            sourceIndex = CIRCLE_LED_COUNT - 1;
        }
        else{
            sourceIndex = sinkIndex - 1;
        }
    }

    clearBuffer();
    for(int i = 0; i < sourceIntensity; i++){
        circleEnableBuffer[sourceIndex*CIRCLE_PWM_DEPTH + i] = portEnableSequence[sourceIndex];
    }
    for(int i = 0; i < sinkIntensity; i++){
        circleEnableBuffer[sinkIndex*CIRCLE_PWM_DEPTH + i] = portEnableSequence[sinkIndex];
    }

    //print clockface
    const uint8_t binaryLedStart = pEndIndex - 1;

    for(int i = 0; i<8; i++){
        if(clockFaceSeconds & (0x1 << i)){
            circleEnableBuffer[(binaryLedStart - i)*CIRCLE_PWM_DEPTH] = portEnableSequence[binaryLedStart-i];
        }
    }

    if(!modState){
        if(clockFaceMultiples > 0){
            PWM_CMP_LEFT = TIMER_TOP/60;
        }
        else{
            PWM_CMP_LEFT = 0;
        }
        
        if(clockFaceMultiples > 1){
            PWM_CMP_RIGHT = TIMER_TOP/60;
        }
        else{
            PWM_CMP_RIGHT = 0;
        }

        if(clockFaceMultiples > 2){
            PWM_CMP_TOP = TIMER_TOP/60;
        }
        else{
            PWM_CMP_TOP = 0;
        }
    }

}
#undef clockFaceSeconds
#undef clockFaceMultiples 
#undef quarterSeconds 
#undef direction 
#undef index 
#undef slowPendulum
#undef slowCounter


#define goToZero animVars[0]
#define stopped animVars[1]
#define currentIntensity animVars[2]
#define rising animVars[3]
const void animatePulsingLight(int first){
    if(first){
        msPerFrame = 200;
        currentIntensity = 1;
        rising = 1;
        goToZero = 0;
        stopped = 0;
        modifiableVars = 2;
        return;
    }

    if(!stopped){
        if(rising){
            currentIntensity++;
            if(currentIntensity >= CIRCLE_PWM_DEPTH){
                rising = 0;
            }
        }
        else{
            currentIntensity--;
            if(currentIntensity <= ((goToZero) ? 0 : 1)){
                rising = 1;
            }
        }
    }

    //draw circle
    clearBuffer();
    for(int i = 0; i < CIRCLE_LED_COUNT; i++){
        for(int j = 0; j < currentIntensity; j++){
            circleEnableBuffer[i*CIRCLE_PWM_DEPTH + j] = portEnableSequence[i];
        }
    }

    //draw corners
    if(!modState){
        PWM_CMP_LEFT = PWM_CMP_RIGHT = PWM_CMP_TOP = currentIntensity*TIMER_TOP/60;
    }

}
#undef currentIntensity 
#undef rising 
#undef goToZero 


//TODO: This is bugged when modifiying direction...

#define direction animVars[0]
#define dotsPlaced animVars[1]
#define thisDotPos animVars[2]
#define fadeoutCounter animVars[3]
#define currentBrightness animVars[4]
#define lastDirection animVars[5]
const void animateStackingDots(int first){
    if(first){
        dotsPlaced = 0;
        thisDotPos = 0;
        direction = 0;
        lastDirection = direction;
        fadeoutCounter = 0;
        currentBrightness = CIRCLE_PWM_DEPTH - 1;
        PWM_CMP_LEFT = PWM_CMP_RIGHT = PWM_CMP_TOP = 0;
        modifiableVars = 1;
        msPerFrame = 50;
        return;
    }

    if(lastDirection != direction){
        thisDotPos = (direction) ? 1 : CIRCLE_LED_COUNT - 1; //reset dot on direction change
    }
    lastDirection = direction;
    
    //draw first
    clearBuffer();
    for(int i = 0; i<dotsPlaced; i++){
        uint8_t ledIndex = (direction) ? (CIRCLE_LED_COUNT-1-i) : (i);
        for(int j = 0; j<currentBrightness; j++){
            circleEnableBuffer[CIRCLE_PWM_DEPTH*ledIndex + j] = portEnableSequence[ledIndex];
        }
    }

    if(dotsPlaced < CIRCLE_LED_COUNT - 1){
        for(int j = 0; j<CIRCLE_PWM_DEPTH; j++){
            circleEnableBuffer[CIRCLE_PWM_DEPTH*thisDotPos+ j] = portEnableSequence[thisDotPos];
        }
    }

    if(!modState){
        uint16_t pwmFrac = currentBrightness*TIMER_TOP/60;
        if(dotsPlaced > 11) PWM_CMP_TOP = pwmFrac;
        if(direction){
            if(dotsPlaced > 5) PWM_CMP_RIGHT = pwmFrac;
            if(dotsPlaced == CIRCLE_LED_COUNT) PWM_CMP_LEFT = pwmFrac;
        }
        else{
            if(dotsPlaced > 5) PWM_CMP_LEFT = pwmFrac;
            if(dotsPlaced == CIRCLE_LED_COUNT) PWM_CMP_RIGHT = pwmFrac;
        }
    }

    //fade out if full
    if(dotsPlaced == CIRCLE_LED_COUNT){
        fadeoutCounter++;
        const uint8_t fadeoutPrescaler = 6;
        if(fadeoutCounter >= fadeoutPrescaler){
            fadeoutCounter = 0;
            if(currentBrightness == 0){
                //restart
                dotsPlaced = 0;
                thisDotPos = (direction) ? 0 : CIRCLE_LED_COUNT - 1;
                currentBrightness = CIRCLE_PWM_DEPTH - 2;
            }
            else{
                currentBrightness--;
            }
        }
    }
    else{
        //if not full, move dot
        uint8_t lastPlace = (direction) ? CIRCLE_LED_COUNT - 1 - dotsPlaced : dotsPlaced;
        if(thisDotPos == lastPlace){
            dotsPlaced++;
            thisDotPos = (direction) ? 0 : CIRCLE_LED_COUNT - 1;
        }
        else{
            thisDotPos+= (direction) ? 1 : -1;
        }
    }

}
#undef direction 
#undef dotsPlaced
#undef thisDotPos
#undef fadeoutCounter 
#undef currentBrightness 



#define markerDir animVars[0]
#define eraserDir animVars[1]
#define markerPrescaler animVars[2]
#define eraserPrescaler animVars[3]
#define markerCounter animVars[4]
#define eraserCounter animVars[5]
#define markerPosition animVars[6]
#define eraserPosition animVars[7]
#define topBits animVars[8]
#define leftBits animVars[9]
#define rightBits animVars[10]
const void animateMarkerEraser(int first){
    if(first){
        msPerFrame = 10;
        modifiableVars = 4;
        markerDir = 0;
        eraserDir = 0;
        markerPrescaler = 15;
        eraserPrescaler = 15;
        animVarsMaximums[2] = animVarsMaximums[3] = 0xff;
        animVarsMinimums[2] = animVarsMinimums[3] = 1;

        markerPosition = 0;
        eraserPosition = CIRCLE_LED_COUNT - 1;
        markerCounter = 0;
        eraserCounter = 0;
        topBits = 0;
        leftBits = 0;
        rightBits = 0;
        clearBuffer();
        return;
    }

    markerCounter++;
    eraserCounter++;

    uint8_t dotIntensity =  CIRCLE_PWM_DEPTH;
    uint8_t markIntensity = 1;

    if(markerCounter >= markerPrescaler){
        markerCounter = 0;
        //remove dot from marker position, leaving a mark
        if(markerPosition != eraserPosition){
            for(int i = markIntensity; i < CIRCLE_PWM_DEPTH; i++){
                circleEnableBuffer[markerPosition*CIRCLE_PWM_DEPTH + i] = 0;
            }
        }

        //increment / decrement marker position
        markerPosition += (markerDir) ? 1 : -1;
        if(markerPosition > 250){
            markerPosition = CIRCLE_LED_COUNT - 1;
        }
        else if(markerPosition == CIRCLE_LED_COUNT){
            markerPosition = 0;
        }

        //place dot at new position
        for(int i = 0; i < CIRCLE_PWM_DEPTH; i++){
            circleEnableBuffer[markerPosition*CIRCLE_PWM_DEPTH + i] = portEnableSequence[markerPosition];
        }
    }


    if(eraserCounter >= eraserPrescaler){
        eraserCounter = 0;
        //remove dot from eraser position, leaving nothing
        if(markerPosition != eraserPosition){
            for(int i = 0; i < CIRCLE_PWM_DEPTH; i++){
                circleEnableBuffer[eraserPosition*CIRCLE_PWM_DEPTH + i] = 0;
            }
        }

        //increment / decrement marker position
        eraserPosition += (eraserDir) ? 1 : -1;
        if(eraserPosition > 250){
            eraserPosition = CIRCLE_LED_COUNT - 1;
        }
        else if(eraserPosition == CIRCLE_LED_COUNT){
            eraserPosition = 0;
        }

        //place dot at new position
        for(int i = 0; i < CIRCLE_PWM_DEPTH; i++){
            circleEnableBuffer[eraserPosition*CIRCLE_PWM_DEPTH + i] = portEnableSequence[eraserPosition];
        }
    }

    //set bits under marker, reset under eraser
    uint8_t markVal = (0x1 << markerPosition % 6);       
    if(markerPosition < 6){
        leftBits |= markVal;
    }
    else if(markerPosition < 12){
        topBits |= markVal;
    }
    else{
        rightBits |= markVal;
    }

    uint8_t eraseVal = ~(0x1 << eraserPosition % 6);
    if(eraserPosition < 6){
        leftBits &= eraseVal;
    }
    else if(eraserPosition< 12){
        topBits &= eraseVal;
    }
    else{
        rightBits &= eraseVal;
    }

    uint8_t topSum = 0;
    uint8_t leftSum = 0;
    uint8_t rightSum = 0;
    for(uint8_t i = 0; i<6; i++){
        topSum += (topBits >> i) & 0x1;
        leftSum += (leftBits >> i) & 0x1;
        rightSum += (rightBits >> i) & 0x1;
    }



    if(!modState){
        PWM_CMP_LEFT = leftSum*TIMER_TOP/60;
        PWM_CMP_TOP = topSum*TIMER_TOP/60;
        PWM_CMP_RIGHT = rightSum*TIMER_TOP/60;
    }

}

#undef markerDir 
#undef eraserDir 
#undef markerPrescaler 
#undef eraserPrescaler 
#undef markerCounter 
#undef eraserCounter 
#undef markerPosition
#undef eraserPosition
#undef topVal 
#undef leftVal
#undef rightVal 
