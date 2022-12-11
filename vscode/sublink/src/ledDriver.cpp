
#include <Arduino.h>

unsigned int ledCircle[16]; //on/off states of leds in the inner circle (3x6), starting from bottom led of the left corner sextuple and going clockwise

#define L0 4
#define L1 5
#define L2 3
#define L3 6
#define L4 7

#define LOW 0
#define HIGH 1
#define Z 2

void setL(unsigned int L, unsigned int value){
    if(L == L2){ //L2 is on B port, hence the exception
        unsigned int bit = 0x1 << L;
        if(value == Z){
            PORTB_DIRCLR = bit;
        }
        else if(value == LOW){
            PORTB_OUTCLR = bit;
            PORTB_DIRSET = bit;
        }
        else if(value == HIGH){
            PORTB_OUTSET = bit;
            PORTB_DIRSET = bit;
        }
    }
    else{ //everything else is on port A
        unsigned int bit = 0x1 << L;
        if(value == Z){
            PORTA_DIRCLR = bit;
        }
        else if(value == LOW){
            PORTA_OUTCLR = bit;
            PORTA_DIRSET = bit;
        }
        else if(value == HIGH){
            PORTA_OUTSET = bit;
            PORTA_DIRSET = bit;
        }
    }
}

//iterates over values saved in ledCircle, flashing respective led on if a 1 is found. 
void displayLedCircle(){
    //tristate everything 
}

void ledDriverInit(){
    for(int i=0;i<16;i++){
        ledCircle[i] = 0;
    }

    

}
