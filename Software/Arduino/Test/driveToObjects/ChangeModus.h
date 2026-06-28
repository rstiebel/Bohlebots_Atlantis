
void changeModus()
{
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  switch(mode)
  {
    case 0: mode = 1; break;
    case 1: mode = 0; break;
  }
}

void changeScoreGoal() {
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