#include <elapsedMillis.h>

#define KICKER_PIN 15
elapsedMillis kickTimer;

void kick(unsigned int time) {
  if(kickTimer > 2000) {
    if(time > 30) time = 30;
    digitalWrite(KICKER_PIN, HIGH);
    delay(time);
    digitalWrite(KICKER_PIN, LOW);
    kickTimer = 0;  
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(KICKER_PIN, OUTPUT);
  digitalWrite(KICKER_PIN, LOW);
}

void loop() {
  kick(20);
  delay(5);
  
}