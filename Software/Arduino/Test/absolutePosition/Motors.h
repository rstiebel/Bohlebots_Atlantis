
void initMotors()
{
  digitalWrite(DRIVE_ENA, LOW);
  
  pinMode(DRIVE_ENA , OUTPUT);
  pinMode(DRIVE1_DIR, OUTPUT);
  pinMode(DRIVE2_DIR, OUTPUT);
  pinMode(DRIVE3_DIR, OUTPUT);
  pinMode(DRIVE4_DIR, OUTPUT);
  pinMode(DRIB_DIR, OUTPUT);

  ledcAttach(DRIVE1_PWM, 5000, 8);
  ledcAttach(DRIVE2_PWM, 5000, 8);
  ledcAttach(DRIVE3_PWM, 5000, 8);
  ledcAttach(DRIVE4_PWM, 5000, 8);
   ledcAttach(DRIB_PWM, 1000, 8);

  ledcWrite(DRIVE1_PWM, 0);
  ledcWrite(DRIVE2_PWM, 0);
  ledcWrite(DRIVE3_PWM, 0);
  ledcWrite(DRIVE4_PWM, 0);
  ledcWrite(DRIB_PWM, 0);
  
}

int spdToPwm(double spd) 
{
 // pwm 25..229 (<10%..<90%) für 0..100
 return (int) (25.5 + (203.0/100.0) * ABS(spd)); 
}

void motor(boolean e, int p1, boolean d1, int p2, boolean d2, int p3, boolean d3, int p4, boolean d4) // Achtung neu !!!
{
  digitalWrite(DRIVE_ENA, HIGH);
  ledcWrite(DRIVE4_PWM, p4);
  ledcWrite(DRIVE3_PWM, p3);
  ledcWrite(DRIVE2_PWM, p2);
  ledcWrite(DRIVE1_PWM, p1);
  
  digitalWrite(DRIVE4_DIR, d4);
  digitalWrite(DRIVE3_DIR, d3);
  digitalWrite(DRIVE2_DIR, d2);
  digitalWrite(DRIVE1_DIR, d1);
}

void drive(double angle, int speed, int rot) 
{
  int p1, p2, p3, p4;               //  PWM Werte
  boolean d1, d2, d3, d4, ena;          //  dirs Werte
  double spd_1, spd_2, spd_3, spd_4; //speeds Werte
  double radmax;                   //speed den schnellsten Rads
  
  if(speed<0) speed=0;
  if(speed>100) speed=100;
  
  if( rot > ( 100-speed) ) rot= 100-speed;  // speed und rot können 100 nicht überschreiten
  if( rot < (-100+speed) ) rot=-100+speed;

  //Lineare Anteile berechnen         Siehe readme.h
  spd_1 = sin(angle + PI/3.0) * speed;   // vorne links
  spd_2 = sin(angle - PI/3.0) * speed;   // vorne rechts
  spd_3 = sin(angle + 3.0*PI/4.0) * speed; // hinten links
  spd_4 = sin(angle - 3.0*PI/4.0) * speed; // hinten rechts

  // auf maximal 100% ziehen
  radmax = ABS(spd_1);
  if (ABS(spd_2)>radmax) radmax= ABS(spd_2);
  if (ABS(spd_3)>radmax) radmax= ABS(spd_3);
  if (ABS(spd_4)>radmax) radmax= ABS(spd_4);
  
  if (radmax > 1) { spd_1 = spd_1/radmax;    spd_2 = spd_2/radmax;    spd_3 = spd_3/radmax;    spd_4 = spd_4/radmax;  } 

  spd_1 = (spd_1*speed)+rot;  
  spd_2 = (spd_2*speed)+rot;  
  spd_3 = (spd_3*speed)+rot;  
  spd_4 = (spd_4*speed)+rot;
  

  p1=spdToPwm(spd_1);  
  p2=spdToPwm(spd_2);  
  p3=spdToPwm(spd_3);  
  p4=spdToPwm(spd_4);

  if (spd_1 < 0 ) d1 = LOW; else d1 = HIGH;
  if (spd_2 < 0 ) d2 = LOW; else d2 = HIGH;  
  if (spd_3 < 0 ) d3 = LOW; else d3 = HIGH;
  if (spd_4 < 0 ) d4 = LOW; else d4 = HIGH;  
  ena = HIGH;

  //Serial.println("p1: " + String(p1) + ", d1: " + String(d1) + ", p2: " + String(p2) + ", d2: " + String(d2) + ", p3: " + String(p3) + ", d3: " + String(d3) + ", p4: " + String(p4) + ", d4: " + String(d4));
  motor(ena, p1, d1 ,p2, d2, p3, d3, p4, d4);

}

void dribbler(int spd) {
  if(spd<0) spd=0;
  if(spd>100) spd=100;

  digitalWrite(DRIVE_ENA, HIGH);
  int p = spdToPwm(spd); 
  ledcWrite(DRIB_PWM, p);
  digitalWrite(DRIB_DIR, HIGH);
}
