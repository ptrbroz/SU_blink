
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

  //buttons
  PORTA_PIN1CTRL = 0x80 | 0x8; //pullup enable, invert
  PORTA_PIN2CTRL = 0x80 | 0x8; //pullup enable
  PORTA_PIN3CTRL = 0x80 | 0x8; //pullup enable


}

void loop(){

  uint8_t buttonStatePreviousLoop = 0;
  uint8_t buttonStateLast = 0;
  uint8_t holdState = 0;
  uint32_t buttonDebounceCounter = 0;

  const uint32_t debounceVal        =  10000;
  const uint32_t holdDelay          = 330000;

  enterAnimation(4);

  while(1){
    buttonDebounceCounter += TCA0_SINGLE_CNT;

    uint8_t buttonStateNow = (PORTA_IN & (0x7 << 1));
    if(buttonStateNow != buttonStatePreviousLoop){
      buttonDebounceCounter = 0;
      holdState = 0;
    }
    buttonStatePreviousLoop = buttonStateNow;

    if(holdState == 0){
      if(buttonDebounceCounter > debounceVal){
        if((!(buttonStateLast & 0x2)) && (buttonStateNow & 0x2)){ //butt 1 press - right
          handleModify(-1);
          holdState = 1;
        }
        else if((!(buttonStateLast & 0x4)) && (buttonStateNow & 0x4)){ //butt 1 press - mid 
          handleModButton();
        }
        else if(!((buttonStateLast & 0x8)) && (buttonStateNow & 0x8)){ //butt 1 press - left
          handleModify(1);
          holdState = 1;
        }
        buttonStateLast  = buttonStateNow;
        buttonDebounceCounter = 0;
      }
    }
    else if(holdState < 3){
      if(buttonDebounceCounter > holdDelay){
        buttonDebounceCounter = 0;
        holdState++;
        //don't do modification on first delay
      }
    }
    else{
      if(buttonDebounceCounter > holdDelay){
        uint8_t multiplier = 1;
        if(holdState < 10){
          multiplier = 3;
        }
        else{
          holdState++;
        }
        buttonDebounceCounter = 0;
        if(buttonStateNow & 0x2){
          handleModify(-1*multiplier);
        }
        else if(buttonStateNow & 0x8){
          handleModify(1*multiplier);
        }
      }
    }

    animate();
    //PORTB_OUT |= 0x1;

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