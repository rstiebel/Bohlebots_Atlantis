#include <EEPROM.h>
struct EEPROM_DATA {
  byte testMode = TESTMODE_NONE;
  byte gameMode = GAMEMODE_STRIKER_BASIC;
  byte neutralPosition = NEUTRAL_POSITION_OFFENSIVE;
  bool scoreOnYellow = true; 
  bool comModUsed = false; 
  bool play = false;
  // add what you want to store here
};

const int EEPROM_Address = 0;
EEPROM_DATA eepromData;

void EEPROM_Init() {
  EEPROM.begin(sizeof(EEPROM_DATA));
}

EEPROM_DATA EEPROM_Read() {
  EEPROM_DATA Data;
  memset(&Data, 0, sizeof(EEPROM_DATA)); // Init data with 0s
  EEPROM.readBytes(EEPROM_Address, &Data, sizeof(EEPROM_DATA));
  return Data;
}

void EEPROM_Write(const EEPROM_DATA& Data) {
  EEPROM.writeBytes(EEPROM_Address, &Data, sizeof(EEPROM_DATA));
  EEPROM.commit();
}