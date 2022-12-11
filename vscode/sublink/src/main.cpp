/*
#include <Arduino.h>
void setup() {
  // put your setup code here, to run once:
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(0, LOW);
  digitalWrite(2, LOW);
  delay(1000);
  digitalWrite(0, HIGH);
  delay(200);
  digitalWrite(0, LOW);
  delay(200);
  digitalWrite(2, HIGH);
  delay(200);
  digitalWrite(2, LOW);
  printf("cool!");
}
*/

#include <ledDriver.h>

int main(){

  ledDriverInit();

}