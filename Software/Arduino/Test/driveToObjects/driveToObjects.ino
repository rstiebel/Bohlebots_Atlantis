#include "Vector.h"
#include "Definitions.h"
#include "ChangeModus.h"
#include "Camera.h"
#include "Motors.h"
#include "i2cHandler.h"
I2CHandler i2cHandler; 

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  initMotors();
  i2cHandler.initI2C();
  i2cHandler.leftModule = 0; //schrottmühle = 3, 2
  i2cHandler.rightModule = 1; // auf den Modulen gilt: rechts = 1 und links = 2  // schrottmühle = 1, 0
  i2cHandler.led(i2cHandler.rightModule, 1, GREEN);
  i2cHandler.led(i2cHandler.rightModule, 2, YELLOW);
  i2cHandler.led(i2cHandler.leftModule, 1, MAGENTA);
  mode = 0;
  driveTimer=0;
}

void printDebug() {
  Serial.println("vGoal");
  vGoal.debug_println();
  Serial.println("vOwnGoal");
  vOwnGoal.debug_println();
  Serial.println("midpoint");
  vMidPoint.debug_println();
  Serial.println("midaxis");
  vMidAxis.debug_println();
}
void Gameplay() {
  if(ballSeen) {
    //driveToPointSoft(vBall, 5, 30);
    Vector vOffset;
    if(midPointValid) {
      vOffset = vMidAxis;
    }
    else if (goalSeen){
      vOffset = vGoal; //Vector::rotate(vGoal, PI);
    }
    else if (ownGoalSeen){
      vOffset = Vector::rotate(vOwnGoal, PI);
    }
    else vOffset = Vector();

    vOffset = vOffset.unit();
    Vector vTarget;
    double a = vBall.getAlpha() * 0.5;
    vOffset.rotate(a);
    double factorBallAngle = (  (ABS(vBall.getAlpha()) > 0.5) ? 1.0 : 0.4  );
    vOffset *= (factorBallAngle * 40);
    vTarget = vBall - vOffset;
    driveToPointSoft(vTarget, 5, 30);
  }
  else {
    driveToMidPoint();
  }
}
void Game() {
  switch(mode) {
    case 0: Gameplay();break;
    case 1: printDebug(); break;
  }
}
void loop() {
  readData();
  COLOR c = OFF;
  switch (mode) {
    case 0: c = GREEN; break;
    case 1: c = WHITE; break;
  }

  if(i2cHandler.button(i2cHandler.rightModule, 1)) {
    if(i2cHandler.button(i2cHandler.leftModule, 2)) {
      changeModus();
    }
    else changePlay();
  }
  if(i2cHandler.button(i2cHandler.rightModule, 2)) changeScoreGoal();
  
  i2cHandler.led(i2cHandler.rightModule, 1, c);
  i2cHandler.led(i2cHandler.rightModule, 2, (scoreOnYellow ? YELLOW : BLUE));
  
  if(play) {
    //drive(-0.5*PI, 20, 0);
    Game();
    //drive(0, 50, 0);
  }
  else {
    drive(0, 0, 0);
  }
  i2cHandler.wait(5);
}