
void initMotors()
{
  digitalWrite(DRIVE_ENA, LOW);
  
  pinMode(DRIVE_ENA , OUTPUT);
  pinMode(DRIB_DIR, OUTPUT);

  ledcAttach(DRIB_PWM, 1000, 8);

  ledcWrite(DRIB_PWM, 0);
}

int spdToPwm(double spd) 
{
 // pwm 25..229 (<10%..<90%) für 0..100
 return (int) (25.5 + (203.0/100.0) * ABS(spd)); 
}

void dribbler(int spd) {
  if(spd<0) spd=0;
  if(spd>100) spd=100;

  digitalWrite(DRIVE_ENA, HIGH);
  int p = spdToPwm(spd); 
  ledcWrite(DRIB_PWM, p);
  digitalWrite(DRIB_DIR, HIGH);
}

void changeModus() {
  if(buttonTimer < 500) return;
  buttonTimer = 0;
  mode = !mode;
}
