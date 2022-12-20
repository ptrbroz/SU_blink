#ifndef ANIMATIONS_H
#define ANIMATIONS_H

//volatile extern uint16_t msPerFrame;

//Checks TCA COUNT if it is time to proceed to next frame of active animation. If yes, modifies circleEnableBuffer and PWM duties
void animate();
 
//Changes active animation to @anim and modifies circleEnableBuffer to contain first frame of @anim.
void enterAnimation(uint8_t anim);

//Called on pressing the middle button, begins modification of active animation, or switches to next value to modify, or leaves mod mode.
void handleModButton();

//Called on pressing right or left button. @value is intended increment / decrement (+-1 on single press, may also be higher if desired) currently selected property of animation
//or to switch to next / previous animation
void handleModify(int value);






#endif