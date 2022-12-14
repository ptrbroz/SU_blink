#ifndef ANIMATIONS_H
#define ANIMATIONS_H

enum animType{
    none,
    runningDot
};


//Checks TCA COUNT if it is time to proceed to next frame of active animation. If yes, modifies circleEnableBuffer and PWM duties
void animate();
 
//Changes active animation to @anim and modifies circleEnableBuffer to contain first frame of @anim.
void enterAnimation(animType anim);


#endif