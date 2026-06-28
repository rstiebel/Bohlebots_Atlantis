
void Game() {
  if(button(rightModule, 1)) changePlay();
  statusBlink(OFF);
  switch(mode)
  {
    case 0: Striker(); break;
    case 1: Keeper(); break;
    case 2: dribbler(100); drive(0, 0, 20); break; //kick(KICK_POWER); break;
    case 3: dribbler(100); break;
    case 4: drive(0, 50, 0); break;
    default: break;
  }
}

void GameSetup() {
  drive(0, 0, 0);
  dribbler(0);

  COLOR c = OFF;
  // ball 
  if (ballSeen) c = ((ABS(vBall.getAlpha()) < 0.05 * PI) ? MAGENTA : WHITE);
  // if (hasBall()) c = GREEN; // TODO
  led(leftModule, 2, c);

  /*if(button(leftModule, 1)) {
    if (button(leftModule, 2)) {
      changeRole();
    }
    else Esp.restart();
  }
  */

  if(button(leftModule, 1)) ESP.restart();
  if(button(rightModule, 2)) changeGoal();
  led(rightModule, 2, (scoreOnYellow ? YELLOW : BLUE));

  if(button(rightModule, 1)) 
  {
    if (button(leftModule, 2)) {
      changeModus();
    }
    else changePlay();
  }

  switch (mode) {
    case 0: c = RED; break; // Striker
    case 1: c = GREEN; break; // Keeper
    case 2: c = BLUE; break; // Kick
    case 3: c = CYAN; break; // Dribbler
    case 4: c = MAGENTA; break; // drive forward
  }
  led(rightModule, 1, c);



}