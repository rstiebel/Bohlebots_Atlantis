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
  i2cHandler.led(i2cHandler.leftModule, 1, BLUE);
  mode = false;
  driveTimer=0;
}

void loop() {
  /*if(i2cHandler.button(i2cHandler.rightModule, 1)) mode = !mode;
  if(mode) {
    i2cHandler.led(i2cHandler.rightModule, 1, RED);
    drive(0, 50, 0);
  }
  else {
    i2cHandler.led(i2cHandler.rightModule, 1, GREEN);
    drive(0, 0, 0);
  }

  i2cHandler.wait(5);*/
  //singleMotor(128, HIGH);

  /*if(driveTimer < 1000) {
    drive(0, 50, 0);
  }
  else drive(0, 0, 0);
  */
  drive(0, 50, 0);
  delay(5);
}
