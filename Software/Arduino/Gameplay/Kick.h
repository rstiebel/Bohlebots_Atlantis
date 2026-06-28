bool lastHasBall = false;

bool hasBall()
{
  return (analogRead(INPUT4) > 2500); 
}

bool changedHasBallStatus() {

  bool status = (lastHasBall == hasBall() ? false : true);
  return status;
}

void initKick() {
  pinMode(KICKER_PIN, OUTPUT);
  digitalWrite(KICKER_PIN, LOW);

  pinMode(INPUT1, INPUT);
  pinMode(INPUT2, INPUT);
  pinMode(INPUT3, INPUT);
  pinMode(INPUT4, INPUT);
}

void kick(unsigned int time) {
  if(kickTimer > 1000) {
    if(time > 30) time = 30;
    digitalWrite(KICKER_PIN, HIGH);
    delay(time);
    digitalWrite(KICKER_PIN, LOW);
    kickTimer = 0;  
  }
}