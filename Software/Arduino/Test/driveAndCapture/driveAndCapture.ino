#include "functions.h"
bool mode = false;
int dir = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
  initMotors();

  pinMode(button,     INPUT_PULLUP);
  mode = false;
  dir = 0;
  Serial.println("kann losgehen");

}

int getRandomExcluding(int currentValue) {
  // Liste der auszuschließenden Werte (aktueller Wert ±1 und Bereichsübergänge)
  int excludedValues[3];
  excludedValues[0] = currentValue;
  excludedValues[1] = (currentValue == 8) ? -7 : currentValue + 1;  // nächster Wert (beachte Bereichsübergang)
  excludedValues[2] = (currentValue == -7) ? 8 : currentValue - 1; // vorheriger Wert (beachte Bereichsübergang)
  
  int randomValue;
  bool isValid;
  
  do {
    // Generiere Zufallszahl zwischen -7 und 8
    randomValue = random(-7, 9); // Obere Grenze ist exklusiv, daher 9
    
    // Prüfe, ob der generierte Wert nicht in den ausgeschlossenen Werten ist
    isValid = true;
    for (int i = 0; i < 3; i++) {
      if (randomValue == excludedValues[i]) {
        isValid = false;
        break;
      }
    }
  } while (!isValid);
  
  return randomValue;
}

 
void loop() {
  int state = digitalRead(button);
  if (state == LOW) mode = !mode;
  bodenUpdate();
  if (mode) 
  {
    if(siehtLinie)
    {
      dir = getRandomExcluding(linienRichtung);
    }
    drive(dir, 30, 0);
  }
  delay(10);
}
