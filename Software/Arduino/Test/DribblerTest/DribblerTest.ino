#include "Definitions.h"
#include "functions.h"
#include "i2cHandler.h"
I2CHandler i2cHandler; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  initMotors();
  i2cHandler.initI2C();
  i2cHandler.leftModule = 0; //schrottmühle = 3, 2
  i2cHandler.rightModule = 1; // auf den Modulen gilt: rechts = 1 und links = 2  // schrottmühle = 1, 0
  i2cHandler.led(i2cHandler.rightModule, 1, GREEN);
  i2cHandler.led(i2cHandler.rightModule, 2, YELLOW);
  i2cHandler.led(i2cHandler.leftModule, 1, MAGENTA);
  mode = false;
}

void loop() {
  if(i2cHandler.button(i2cHandler.rightModule, 1)) changeModus();
  if(i2cHandler.button(i2cHandler.leftModule, 1)) ESP.restart();
  if (mode) dribbler(100);
  else dribbler(0);
  i2cHandler.wait(5);
}