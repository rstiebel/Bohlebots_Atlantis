
void changeModus()
{
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  switch(mode)
  {
    case 0: mode = 1; break; // Striker
    case 1: mode = 2; break; // Keeper
    case 2: mode = 3; break; // Kick
    case 3: mode = 4; break; // Dribbler
    case 4: mode = 0; break; // Drive forward
  }
}

void changeGoal() {
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  scoreOnYellow = !scoreOnYellow;
}

void changePlay()
{
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  play = !play;
}