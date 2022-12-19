
#include <Arduino.h>
#include <ledDriver.h>
#include <animations.h>
#include <common.h>

int main(){
  setup();
  while(1){
    loop();
  }
}


void setup(){
  clearBuffer();

  //system clock
  CCP = 0xd8; //allow self-programming
  CLKCTRL_MCLKCTRLA = 0; //clock source = 16MHz oscillator
  CCP = 0xd8; //allow self-programming
  CLKCTRL_MCLKCTRLB = 0x1 | (0x0 << 1); //clock prescaler 2x ->  clock speed 8M

  //pwm setup
  TCA0_SINGLE_CTRLA = (0x1) | (0x3 << 1); //enable, prescaler
  TCA0_SINGLE_CTRLB = (0x7 << 4); //enable compare channels 0 1 2 
  TCA0_SINGLE_CTRLB |= 0x3; // single slope pwm
  TCA0_SINGLE_PER = TIMER_TOP; //set top value
  PORTB_DIR |= (0x1|0x2|0x4); //set PB0 1 2 as output -- pwm pins

}

void loop(){


  enterAnimation(runningDot);

  while(1){


    animate();
    //blinkEdge(1,1,1, 1000);


    for(int i = 0; i<CIRCLE_LED_COUNT*CIRCLE_PWM_DEPTH; i+=1){
      uint8_t enableVal = circleEnableBuffer[i];
      uint8_t outVal = portOutSequence[i/CIRCLE_PWM_DEPTH];

      //handle L2 and remove from variabl
      if(enableVal & (1<<L2)){
        PORTB_OUTSET = (outVal & (1<<L2));
        PORTB_OUTCLR = (1<<L2)^(outVal & (1<<L2));
        PORTB_DIRSET = 1<<L2;
        enableVal  &= ~(1<<L2);
        outVal     &= ~(1<<L2);
      }
      else{
        PORTB_DIRCLR = 1<<L2;
      }

      //handle the rest
      PORTA_OUT = outVal;
      PORTA_DIR = enableVal;



    }
  }



}