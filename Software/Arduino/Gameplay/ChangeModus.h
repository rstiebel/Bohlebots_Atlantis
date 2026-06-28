
void changeTestModus()
{
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  switch(eepromData.testMode)
  {
    case TESTMODE_NONE: eepromData.testMode = TESTMODE_KICK; break;
    case TESTMODE_KICK: eepromData.testMode = TESTMODE_DRIBBLER; break;
    case TESTMODE_DRIBBLER: eepromData.testMode = TESTMODE_DRIVE_STRAIGHT; break;
    case TESTMODE_DRIVE_STRAIGHT: eepromData.testMode = TESTMODE_DRIVE_RANDOM; break;
    case TESTMODE_DRIVE_RANDOM: eepromData.testMode = TESTMODE_MIDAXIS; break;
    case TESTMODE_MIDAXIS: eepromData.testMode = TESTMODE_AIMED_KICK; break;
    case TESTMODE_AIMED_KICK: eepromData.testMode = TESTMODE_NONE; break;
    default: eepromData.testMode = TESTMODE_NONE; break;
  }
  EEPROM_Write(eepromData);
}

void changeGameModus()
{
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  switch(eepromData.gameMode)
  {
    case GAMEMODE_STRIKER_BASIC: eepromData.gameMode = GAMEMODE_STRIKER_WINGER; break;
    case GAMEMODE_STRIKER_WINGER: eepromData.gameMode = GAMEMODE_STRIKER_DYNAMIC; break;
    case GAMEMODE_STRIKER_DYNAMIC: eepromData.gameMode = GAMEMODE_STRIKER_FLANK; break;
    case GAMEMODE_STRIKER_FLANK: eepromData.gameMode = GAMEMODE_STRIKER_DYNAMIC_OBJECTS; break;
    case GAMEMODE_STRIKER_DYNAMIC_OBJECTS: eepromData.gameMode = GAMEMODE_KEEPER_BASIC; break;
    case GAMEMODE_KEEPER_BASIC: eepromData.gameMode = GAMEMODE_KEEPER_BLOCKING; break;
    case GAMEMODE_KEEPER_BLOCKING: eepromData.gameMode = GAMEMODE_STRIKER_BASIC; break;
    default: eepromData.gameMode = GAMEMODE_STRIKER_BASIC; break;
  }
  EEPROM_Write(eepromData);
}

void changeNeutralPosition()
{
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  switch(eepromData.neutralPosition)
  {
    case NEUTRAL_POSITION_OFFENSIVE: eepromData.neutralPosition = NEUTRAL_POSITION_DEFENSIVE; break;
    case NEUTRAL_POSITION_DEFENSIVE: eepromData.neutralPosition = NEUTRAL_POSITION_OFFENSIVE; break;
    default: eepromData.neutralPosition = NEUTRAL_POSITION_OFFENSIVE; break;
  }
  EEPROM_Write(eepromData);
}

void changeGoal() {
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  eepromData.scoreOnYellow = !eepromData.scoreOnYellow;
  EEPROM_Write(eepromData);
}

void changeComModUsed()
{
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  eepromData.comModUsed = !eepromData.comModUsed;
  EEPROM_Write(eepromData);
}

void changePlay()
{
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  switchTimer = 200;
  eepromData.play = !eepromData.play;
  EEPROM_Write(eepromData);
}