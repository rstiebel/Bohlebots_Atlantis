#include "Vector.h"
#include "Definitions.h"
#include "functions.h"

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Arduino bereit zum Empfangen");
}

void loop() {
  readData();
  Serial.println("Ball");
  vBall.debug_println();
  Serial.println("Yellow");
  vYellow.debug_println();
  Serial.println("Blue");
  vBlue.debug_println();
}