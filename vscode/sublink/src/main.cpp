
#include <Arduino.h>
#include <ledDriver.h>
#include <animations.h>

int main(){
  setup();
  while(1){
    loop();
  }
}


void setup(){
  clearBuffer();

  //pwm setup
  TCA0_SINGLE_CTRLA = (0x1) | (0x4 << 1); //enable, prescaler
  TCA0_SINGLE_CTRLB = (0x7 << 4); //enable compare channels 0 1 2 
  TCA0_SINGLE_CTRLB |= 0x3; // single slope pwm
  TCA0_SINGLE_PER = 255; //set top value
  PORTB_DIR |= (0x1|0x2|0x4); //set PB0 1 2 as output -- pwm pins

}

void loop(){

  
  //TCA0_SINGLE_CMP0 = 10; //top
  //TCA0_SINGLE_CMP1 = 10; //left
  //TCA0_SINGLE_CMP2 = 10; //right
  

  enterAnimation(runningDot);

  while(1){


    animate();


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