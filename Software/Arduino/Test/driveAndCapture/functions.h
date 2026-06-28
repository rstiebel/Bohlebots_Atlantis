#include <Wire.h>

#define DRIVE_DIS   2

#define DRIVE1_PWM 12
#define DRIVE1_DIR 13

#define DRIVE2_PWM 27
#define DRIVE2_DIR 14

#define DRIVE3_PWM 26
#define DRIVE3_DIR 25

#define DRIVE4_PWM 19
#define DRIVE4_DIR 18

#define button 0

#ifndef BODEN_H
#define BODEN_H

#define BODEN_ADD 0x30

#define SDA 21
#define SCL 22

#define ABS(X) ((X) < 0 ? -(X) : (X))

bool siehtLinie     = false;
bool linienSprung   = false;
bool linieEindeutig = false;
int  linienRichtung = 0;

int bodenUpdate() {
  Wire.requestFrom(BODEN_ADD, 1);
  // prüfen ob antwort kommt
  if (!Wire.available()) {
    // werte auf standart setzen und fehler zurueckgeben
    siehtLinie     = false;
    linienSprung   = false;
    linieEindeutig = false;
    linienRichtung = 0;
    return -1;
  }
  // paket lesen
  byte paket = Wire.read();

  // werte updaten
  siehtLinie     = paket & 64;
  linienSprung   = paket & 32;
  linieEindeutig = paket & 16;
  linienRichtung = (paket % 16) - 7;

  Serial.println("siehtLinie: " + String(siehtLinie));
  Serial.println("linienSprung: " + String(linienSprung));
  Serial.println("linieEindeutig: " + String(linieEindeutig));
  Serial.println("linienRichtung: " + String(linienRichtung));
  
  // OK zurueckgeben
  return 0; 
}

#endif // BODEN_H


void initMotors()
{
  digitalWrite(DRIVE_DIS , LOW   );
  
  pinMode     (DRIVE_DIS , OUTPUT);
  pinMode     (DRIVE1_DIR, OUTPUT);
  pinMode     (DRIVE2_DIR, OUTPUT);
  pinMode     (DRIVE3_DIR, OUTPUT);
  pinMode     (DRIVE4_DIR, OUTPUT);

  ledcAttach(DRIVE1_PWM, 1000, 8);
  ledcAttach(DRIVE2_PWM, 1000, 8);
  ledcAttach(DRIVE3_PWM, 1000, 8);
  ledcAttach(DRIVE4_PWM, 1000, 8);


  ledcWrite(DRIVE1_PWM, 0);
  ledcWrite(DRIVE2_PWM, 0);
  ledcWrite(DRIVE3_PWM, 0);
  ledcWrite(DRIVE4_PWM, 0);

}

double myAbs(double d)
{
  if (d<0) return -d;
  return d;
}

int spdToPwm(double spd) 
{
 //pwm 25..229 (<10%..<90%) für 0..100
 return (int) (25.5 + (203/100)*myAbs(spd)); 
}


void motor(boolean e, int p1, boolean d1, int p2, boolean d2, int p3, boolean d3, int p4, boolean d4) // Achtung neu !!!
{
  digitalWrite(DRIVE_DIS, HIGH);

  ledcWrite(DRIVE1_PWM, p1);
  ledcWrite(DRIVE2_PWM, p3);
  ledcWrite(DRIVE3_PWM, p2);
  ledcWrite(DRIVE4_PWM, p4);
  
  digitalWrite(DRIVE1_DIR, d1);
  digitalWrite(DRIVE2_DIR, d3);
  digitalWrite(DRIVE3_DIR, d2);
  digitalWrite(DRIVE4_DIR, d4);
}


void drive(int dir, int speed, int rot) 
{
  int p1, p2, p3, p4;               //  PWM Werte
  boolean d1, d2, d3, d4, ena;          //  dirs Werte
  double spd_1, spd_2, spd_3, spd_4; //speeds Werte
  double winkel = dir * PI / 180.0;             //dir in Radiant
  double radmax;                   //speed den schnellsten Rads
  
  //Parameter fixen
  rot = -rot;                  //rechts ist plus...
 
  if(speed<0) speed=0;
  if(speed>100) speed=100;
  //dir=dir % 8;
  
  if( rot > ( 100-speed) ) rot= 100-speed;  // speed und rot können 100 nicht überschreiten
  if( rot < (-100+speed) ) rot=-100+speed;

  //Lineare Anteile berechnen         Siehe readme.h
  spd_3 = sin(winkel + PI/4.0) * speed;   // vorne rechts
  spd_2 = sin(winkel - PI/4.0) * speed;   // vorne links
  spd_4 = sin(winkel + 3.0*PI/4.0) * speed; // hinten rechts
  spd_1 = sin(winkel - 3.0*PI/4.0) * speed; // hinten links

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

  if (spd_1 > 0 ) d1 = LOW; else d1 = HIGH;
  if (spd_2 > 0 ) d2 = LOW; else d2 = HIGH;  
  if (spd_3 > 0 ) d3 = LOW; else d3 = HIGH;
  if (spd_4 > 0 ) d4 = LOW; else d4 = HIGH;  
  ena = HIGH;

  //Serial.println("p1: " + String(p1) + ", d1: " + String(d1) + ", p2: " + String(p2) + ", d2: " + String(d2) + ", p3: " + String(p3) + ", d3: " + String(d3) + ", p4: " + String(p4) + ", d4: " + String(d4));
  motor(ena, p1, d1 ,p2, d2, p3, d3, p4, d4);

}


