#include "Vector.h"
#include "Definitions.h"
#include "AntiReset.h"
#include "ChangeModus.h"
#include "Motors.h"
#include "Camera.h"
#include "Line.h"
#include "Buttons.h"
#include "Rotations.h"
#include "Kick.h"
#include "Communication.h"
#include "BotPosition.h"
#include "ObjectHelperFunctions.h"
#include "Drive.h"
#include "StrikerHelperFunctions.h"
#include "Keeper.h"
#include "Striker.h"
#include "StrikerVariations.h"
#include "Game.h"
#include "Debug.h"

// To search for every value that depends on the field: TODO: could be different on another field 

/*
TODO:
- Strker
-> deutlich schneller
-> computeSpd abhängig von der distanz zum Mittelpunkt
-> kick checken

- aktuelle Werte der Fahrlogik auf das neue System anpassen
- neue Spielmodi einführen 
-> statt modus = 0, 1... enum 
-> überlegen welche Spielzüge man wo haben möchte 
*/

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  // Serial2.setTimeout(10); // 10ms Timeout
  
  Wire.begin(SDA, SCL);
  initMotors();
  initI2C();
  initKick();

  EEPROM_Init();
  eepromData = EEPROM_Read();

  initCom();
}

void loop() {
  readData();
  bodenUpdate();
  comUpdate();
  
  bool debug = Serial.available() > 0;
  while(Serial.available() > 0) {
    Serial.read(); // buffer leeren
  }
  if(debug) debugPrint();

  static bool lastInput3 = 0;
  if (lastInput3 != digitalRead(INPUT3) && eepromData.comModUsed) changePlay();
  lastInput3 = digitalRead(INPUT3);

  if(eepromData.play) Game();
  else {
    GameSetup();
  }
  wait(5);

}