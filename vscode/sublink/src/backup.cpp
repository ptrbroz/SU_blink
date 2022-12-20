

//Needs PWM_DEPTH at least 6!!

/*
#define msElapsed animVars[1]
#define clockFaceSeconds animVars[2]
#define clockFaceMultiples animVars[3]
#define direction animVars[4]
#define travelIndex animVars[5]
#define quarterSeconds animVars[6]
#define bigMsElapsed animVars[7]
#define dominantLedPos animVars[8]
#define dominantLedValue animVars[9]

//not gonna work, main loop takes ~ 2.71 ms with pwmdept 5
const void FLAWEDanimatePendulumClock(int first, int mod, int delta){
    if(first || mod){
        //no modifications allowed on this, so just reset on mod 
        msElapsed = 0;
        clockFaceSeconds = 0;
        clockFaceMultiples = 0;
        travelIndex = 0;
        quarterSeconds = 0;
        direction = 1;
        bigMsElapsed = 0; 
        msPerFrame = 1;
        dominantLedPos = 5;
        dominantLedValue = 6;
        return;
    }
    if(delta != 0){
        return;
    }

    //pendulum LUT from 90° to 0°. Sums to 250 = 1/4 second
    const uint8_t travelTimes[27] = {43, 18, 14, 12, 11, 10, 9, 9, 8, 8, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
    const uint8_t ttMax = sizeof(travelTimes) - 1;

    msElapsed++;
    bigMsElapsed++; //sanity check

    if(msElapsed >= travelTimes[travelIndex]){
        //PORTB_OUT |= 0x1;
        int sd = (direction) ? 1 : -1;
        int nextIndex = travelIndex + sd;
        msElapsed = 0;
        if(nextIndex < 0){
            nextIndex = 0;
            direction = !direction;
            quarterSeconds++;
            if(quarterSeconds >= 4){
                quarterSeconds = 0;
            }
        }
        else if(nextIndex > ttMax){
            nextIndex = ttMax;
            direction = !direction;
            quarterSeconds++;
            if(quarterSeconds == 1){
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
        }

        travelIndex = nextIndex;
        //travelIndex, direction etc should be correct now.

        uint8_t m = travelIndex%6;
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
            sourceIndex = pStartIndex - travelIndex / 6;
            if(sourceIndex == 0){
                sinkIndex = CIRCLE_LED_COUNT-1;
            }
            else{
                sinkIndex = sourceIndex - 1;
            }
        }
        else if(quarterSeconds == 1){
            //upswing from 17 to 13
            int d = (travelIndex / 6) - 6;
            sinkIndex = (CIRCLE_LED_COUNT - 1) + d;
            if(sinkIndex == CIRCLE_LED_COUNT - 1){
                sourceIndex = 0;
            }
            else{
                sourceIndex = sinkIndex + 1;
            }
        }
        else if(quarterSeconds == 2){
            //downswing from 13 to 17
            sourceIndex = pEndIndex + travelIndex / 6;
            if(sourceIndex == CIRCLE_LED_COUNT){
                sinkIndex = 0;
            }
            else{
                sinkIndex = sourceIndex + 1;
            }
        }
        else if(quarterSeconds == 3){
            //upswing from 0 to 4
            int d = (travelIndex / 6) - 6;
            sinkIndex = 0 - d;
            if(sinkIndex == 0){
                sourceIndex = CIRCLE_LED_COUNT - 1;
            }
            else{
                sourceIndex = sinkIndex - 1;
            }
        }

        //source and sink indices and intensities determined, now to render them:
        clearBuffer();
        for(int i = 0; i < sourceIntensity; i++){
            circleEnableBuffer[sourceIndex*CIRCLE_PWM_DEPTH + i] = portEnableSequence[sourceIndex];
        }
        for(int i = 0; i < sinkIntensity; i++){
            //circleEnableBuffer[sinkIndex*CIRCLE_PWM_DEPTH + i] = portEnableSequence[sinkIndex];
        }
    
        //todo draw clockface
    }
    //PORTB_OUT ^= (0x1);
}


*/