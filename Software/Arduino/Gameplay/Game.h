
void Game() {
  if(button(rightModule, BUTTON_RIGHT)) {
    changePlay();
    return;
  }  
  /*
  COLOR c = OFF;

  if (ballSeen) c = ((ABS(vBall.getAlpha()) < 0.05 * PI) ? MAGENTA : WHITE);
  if (hasBall()) c = GREEN; // TODO
  led(leftModule, BUTTON_LEFT, c);
  */

  /*
  led(leftModule, BUTTON_LEFT, OFF);
  led(leftModule, BUTTON_RIGHT, OFF);
  led(rightModule, BUTTON_LEFT, OFF);
  led(rightModule, BUTTON_RIGHT, OFF);
  */
  
  statusBlink(OFF);
  switch (eepromData.testMode) {
    case TESTMODE_NONE:
      switch(eepromData.gameMode)
      {
        case GAMEMODE_STRIKER_BASIC: StrikerBasic(); break;
        case GAMEMODE_STRIKER_WINGER: StrikerWinger(); break;
        case GAMEMODE_STRIKER_DYNAMIC: StrikerDynamic(); break;
        case GAMEMODE_STRIKER_FLANK: StrikerFlank(); break;
        case GAMEMODE_STRIKER_DYNAMIC_OBJECTS: StrikerDynamicObjects(); break;
        case GAMEMODE_KEEPER_BASIC: KeeperBasic(); break;
        case GAMEMODE_KEEPER_BLOCKING: KeeperBlocking(); break;
        default: break;
      }
      break;
    case TESTMODE_KICK: kick(KICK_POWER); break;
    case TESTMODE_DRIBBLER: DRIBBLER_ON; break;
    case TESTMODE_DRIVE_STRAIGHT: drive(0, 50, 0); break;
    case TESTMODE_DRIVE_RANDOM: driveRandom(); break;
    case TESTMODE_MIDAXIS: drive(0, 0, selectRotation(ROTATION_TARGET_MIDAXIS)); break;
    case TESTMODE_AIMED_KICK: testAimedKick(); break;
  }
}

void GameSetup() {
  drive(0, 0, 0);
  DRIBBLER_OFF;

  COLOR c = OFF;
  // ball 
  if (ballSeen) c = ((ABS(vBall.getAlpha()) < 0.05 * PI) ? MAGENTA : WHITE);
  if (hasBall()) c = GREEN; // TODO
  if (!ballSeen && enemySeen) c = RED;
  led(leftModule, BUTTON_LEFT, c);

  bool shiftPressed = button(leftModule, BUTTON_LEFT);
  if (shiftPressed) {
    // Check buttons
    bool toggleCommunication = button(leftModule, BUTTON_RIGHT);
    bool switchTestMode = button(rightModule, BUTTON_LEFT);
    bool switchGameMode = button(rightModule, BUTTON_RIGHT);
    // Update values
    if (toggleCommunication) {
      changeComModUsed();
    }
    if (switchTestMode) {
      changeTestModus();
    }
    if (switchGameMode) {
      changeGameModus();
    }
    // Show new values
    c = (eepromData.comModUsed ? GREEN : OFF);
    led(leftModule, BUTTON_RIGHT, c);
    switch (eepromData.testMode) {
      case TESTMODE_NONE: c = OFF; break;
      case TESTMODE_KICK: c = BLUE; break;
      case TESTMODE_DRIBBLER: c = CYAN; break;
      case TESTMODE_DRIVE_STRAIGHT: c = MAGENTA; break;
      case TESTMODE_DRIVE_RANDOM: c = WHITE; break;
      case TESTMODE_MIDAXIS: c = GREEN; break;
      case TESTMODE_AIMED_KICK: c = RED; break;
    }
    led(rightModule, BUTTON_LEFT, c);
    switch(eepromData.gameMode)
    {
      case GAMEMODE_STRIKER_BASIC: c = RED; break;
      case GAMEMODE_STRIKER_WINGER: c = BLUE; break;
      case GAMEMODE_STRIKER_DYNAMIC: c = WHITE; break; 
      case GAMEMODE_STRIKER_FLANK: c = MAGENTA; break;
      case GAMEMODE_STRIKER_DYNAMIC_OBJECTS: c = CYAN; break;
      case GAMEMODE_KEEPER_BASIC: c = GREEN; break;
      case GAMEMODE_KEEPER_BLOCKING: c = YELLOW; break;
    }
    led(rightModule, BUTTON_RIGHT, c);
  }
  else {
    // Check buttons
    bool switchNeutralPosition = button(leftModule, BUTTON_RIGHT);
    bool switchGoal = button(rightModule, BUTTON_LEFT);
    bool togglePlay = button(rightModule, BUTTON_RIGHT);
    // Update values
    if (switchNeutralPosition) {
      changeNeutralPosition();
    }
    if (switchGoal) {
      changeGoal();
    }
    if (togglePlay) {
      changePlay();
    }
    // Show new values
    switch (eepromData.neutralPosition) {
      case NEUTRAL_POSITION_OFFENSIVE: c = RED; break;
      case NEUTRAL_POSITION_DEFENSIVE: c = GREEN; break;
    }
    led(leftModule, BUTTON_RIGHT, c);
    c = (eepromData.scoreOnYellow ? YELLOW : BLUE);
    led(rightModule, BUTTON_LEFT, c);
    c = (eepromData.comModUsed ? GREEN : OFF);
    led(rightModule, BUTTON_RIGHT, c);
    switch(eepromData.gameMode)
    {
      case GAMEMODE_STRIKER_BASIC: c = RED; break;
      case GAMEMODE_STRIKER_WINGER: c = BLUE; break;
      case GAMEMODE_STRIKER_DYNAMIC: c = WHITE; break; 
      case GAMEMODE_STRIKER_FLANK: c = MAGENTA; break;
      case GAMEMODE_STRIKER_DYNAMIC_OBJECTS: c = CYAN; break;
      case GAMEMODE_KEEPER_BASIC: c = GREEN; break;
      case GAMEMODE_KEEPER_BLOCKING: c = YELLOW; break;
    }
    if (eepromData.testMode != TESTMODE_NONE) c = OFF;
    led(rightModule, BUTTON_RIGHT, c); 
  }
}