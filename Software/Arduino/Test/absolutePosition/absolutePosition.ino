#include "Vector.h"
#include "Definitions.h"
#include "ChangeModus.h"
#include "AntiReset.h"
#include "Motors.h"
#include "Camera.h"
#include "Line.h"
#include "Buttons.h"
#include "Bluetooth.h"
#include "Kick.h"
#include "Drive.h"
#include "Striker.h"
#include "Keeper.h"
#include "Game.h"
#include "Debug.h"
#include "AbsolutePosition.h"

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Wire.begin(SDA, SCL);
  initMotors();
  initI2C();
  //initKick();
  mode = 0;
}

void loop() {
  readData();
  bodenUpdate();
  
  Serial.println("absolutePos vGoal: ");
  Vector vBotAbsPosGoal = computeAbsolutePos(vGoal, vGoalAbsolutePos);
  vBotAbsPosGoal.debug_println();

  Serial.println("absolutePos vOwnGoal: ");
  Vector vBotAbsPosOwnGoal = computeAbsolutePos(vOwnGoal, vOwnGoalAbsolutePos);
  vBotAbsPosOwnGoal.debug_println();

  Serial.println("absolutePos zusammen: ");
  Vector vBotAbsPos = (vBotAbsPosGoal + vBotAbsPosOwnGoal) / 2.0;
  vBotAbsPos.debug_println();

  if(play) Game();
  else {
    GameSetup();
  }
  wait(5);
}