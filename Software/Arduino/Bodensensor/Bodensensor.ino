#include "Definitions.h"


void onRequest() { 
  Wire.write(package, 3);
}

void onReceive(int len) {
  while(Wire.available()) {
    Serial.write(Wire.read());
  }
}

void setup() {
  pinMode(led_addr, OUTPUT);
  pinMode(add0, OUTPUT);
  pinMode(add1, OUTPUT);
  pinMode(add2, OUTPUT);

  Serial.begin(115200);
  digitalWrite(led_addr, LOW);

  Wire.setPins(SDA, SCL);
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  Wire.begin((uint8_t)I2C_DEV_ADDR);
}

/*
1. sesorread
-> arr mit 32 Feldern -> wenn Linie erkannt wird 1 gespeichert an der Stelle des entsprechenden Sensors, ansonsten 0
2. Wolkenanalyse
3. MidpointOfCloud(int Anfang, int Ende)
-> Mittelwert zwischen Anfang und Ende berechem 
4. MidpointToRad(int midpoint)
-> angle in rad berechnen (calculateAngle())
5. wiederhole 3 und 4 für die anderem Vektoren
6. mittleren Winkel berechnen 
-> alle Winkel nehmen und den mittleren Winkel berechnen
-> wir übergeben einen array an Winkeln
7. createVector() 
8. sendResults()
-> x und y Koordinaten senden

nice to have:
-> Länge vom Vektor (wie weit ist die Linie unterm bot) -> Winkel zwischen Vektoren verwendne: je größer desto weiter stehen wir auf der Linie 
*/


int bound_direction(int d) {
  while(d >  8) d -= 16;
  while(d < -7) d += 16;
  return d;
}

int vecToDir(Vector v) {
  // double alpha_adjusted = v.getAlpha() - (PI / 2); falls die diren um 90 grad gedreht werden müssen
  double d = ((v.getAlpha()*180)/PI)/22.5;
  Serial.println("d in deg: " + String(d*22.5));
  int di = 0;
  if(d>0) di = int(d+0.5);
  if(d<0) di = int(d-0.5);
  return bound_direction(di);
}

double averageRad(double radians[], int cloudCount) {
  double sumSin = 0.0;
  double sumCos = 0.0;

  for (int i = 0; i < cloudCount; i++) {
    sumSin += sin(radians[i]);
    sumCos += cos(radians[i]);
  }

  double meanAngle = atan2(sumSin / cloudCount, sumCos / cloudCount); // TODO: eventuell x und y tauschen, da x und y Achse im bot vertauscht sind

  // Ergebnis in 0...2π verschieben, falls negativ
  if (meanAngle < 0) meanAngle += 2.0 * M_PI; // TODO: eventuell weglassen für minus Werte

  // Optional runden (auf 2 Nachkommastellen)
  return round(meanAngle * 100.0) / 100.0;
}

double calculateMidpointOfCloud(double start, double end) {
  const double TWO_TIMES_PI = 2.0 * M_PI; // 360 Grad

  // fmod = modulo mit Fließkommazahlen

  // Unterschied auf dem Kreis -> immer positiv 
  double diff = fmod(end - start + TWO_TIMES_PI, TWO_TIMES_PI);

  // Mittelpunkt der cloud entlang des Kreisbogens 
  return fmod(start + diff / 2.0, TWO_TIMES_PI);

}


void MidpointInRad(const Cloud clouds[], int cloudCount, double midpoints[]) {
  for(int i = 0; i < cloudCount; i++) {
    double startRad = sensors[clouds[i].start].alpha;
    double endRad = sensors[clouds[i].end].alpha;
    midpoints[i] = calculateMidpointOfCloud(startRad, endRad);
  }
}


void mergeClouds(Cloud clouds[], int &cloudCount) {
  if (cloudCount < 2) return; // nur eine Cloud, nichts zu mergen

  Cloud merged;
  merged.start = clouds[cloudCount - 1].start;
  merged.end = clouds[0].end;

  clouds[0] = merged;
  cloudCount--; // aus zwei Clouds wird eine
}

void areCloudsConnected(Cloud clouds[], int &cloudCount) {
  if (cloudCount < 2) return; // nur eine Cloud, nichts zu prüfen

  int zerosBetween = 0;
  int i = (clouds[cloudCount - 1].end + 1) % numOfSensors; // die stelle wo die letzte cloud geendet ist + 1

  while (i != clouds[0].start) {
      if (!sensors[i].line) {
          zerosBetween++;
          if (zerosBetween >= 2) return; // Clouds sind getrennt
      } 
      else zerosBetween = 0; // Reset bei 1
      i = (i + 1) % numOfSensors;
  }

  // Wenn weniger als 2 Nullen → zusammenführen
  if (zerosBetween < 2) {
      mergeClouds(clouds, cloudCount);
  }
}

void findClouds(Cloud clouds[], int &cloudCount) {
  cloudCount = 0;
  int start = -1;          // -1 = keine Cloud aktiv
  int zeros_in_a_row = 0;

  for (int i = 0; i < numOfSensors; i++) {
      if (sensors[i].line) {
          if (start == -1) start = i; // neue Cloud beginnt
          zeros_in_a_row = 0;
      } 
      else { // es wurde keine Linie gefunden
          zeros_in_a_row++;
          if (zeros_in_a_row >= 2 && start != -1) {  
              clouds[cloudCount].start = start;
              clouds[cloudCount].end = i - 2;
              cloudCount++;
              start = -1; // Cloud beendet
          }
      }
  }

  // Letzte Cloud speichern, falls bis zum Ende
  if (start != -1) {
      clouds[cloudCount].start = start;
      clouds[cloudCount].end = numOfSensors - 1;
      cloudCount++;
  }
  areCloudsConnected(clouds, cloudCount);
}

int calculateMidRadius() {
  double sumX = 0;
  double sumY = 0;
  int countNumOfRadius = 0;
  for(int i = 0; i < numOfSensors; i++) {
    if(sensors[i].line) {
      sumX += sensors[i].radius * cos(sensors[i].alpha);
      sumY += sensors[i].radius * sin(sensors[i].alpha);
      countNumOfRadius++;
    }
  }
  if(countNumOfRadius == 0) {
    return 0;
  }
  double midX = sumX / countNumOfRadius;
  double midY = sumY / countNumOfRadius;
  return sqrt(midX * midX + midY * midY); 
}


void setMultiplexerInput(int adresse) 
{
  /*
  INPUT: integer 0-7
  OUTPUT: set address of channel to read
  -> Multiplexer hat 8 Kanäle und muliplexer muss wissen, welchen er auslesen soll. Dafür wird ein binär code von 0-7 über drei verschiedene Pins übergeben
  */
  if ((adresse & 1) == 1) {
    digitalWrite(add0, HIGH); 
  } else {
    digitalWrite(add0, LOW);
  }
  if ((adresse & 2) == 2) {
    digitalWrite(add1, HIGH);
  } else {
    digitalWrite(add1, LOW);
  }
  if ((adresse & 4) == 4) {
    digitalWrite(add2, HIGH);
  } else {
    digitalWrite(add2, LOW);
  }
  delayMicroseconds(60); // damit der multiplexer umschalten kann
}

void sensorRead() {
  /*
  save 1 for Line seen and 0 for not seen
  OUTPUT: arr -> array of 32 for each sensor if line was detected or not
  */
  lineSeen = false;
  for(int i = 0; i < numOfSensors; i++) {
    sensors[i].line = false;
  }
  // die Richtungen sind von oben betrachtet -> auf den bot von oben schauen
  setMultiplexerInput(0); 
    sensors[0].line  = (analogRead(arrSensor[0]) > threshold);
    sensors[16].line = (analogRead(arrSensor[1]) > threshold);
    sensors[32].line = (analogRead(arrSensor[2]) > threshold);
    sensors[3].line  = (analogRead(arrSensor[3]) > threshold);
    sensors[19].line = (analogRead(arrSensor[4]) > threshold);
    sensors[33].line = (analogRead(arrSensor[5]) > threshold);
    
  setMultiplexerInput(1);  
    sensors[1].line  = (analogRead(arrSensor[0]) > threshold);
    sensors[17].line = (analogRead(arrSensor[1]) > threshold);
    sensors[34].line = (analogRead(arrSensor[2]) > threshold);
    sensors[7].line  = (analogRead(arrSensor[3]) > threshold);
    sensors[21].line = (analogRead(arrSensor[4]) > threshold);
    sensors[35].line = (analogRead(arrSensor[5]) > threshold);
    
    
  setMultiplexerInput(2);  
    sensors[2].line  = (analogRead(arrSensor[0]) > threshold);
    sensors[18].line = (analogRead(arrSensor[1]) > threshold);
    sensors[38].line = (analogRead(arrSensor[2]) > threshold);
    sensors[11].line = (analogRead(arrSensor[3]) > threshold);
    sensors[22].line = (analogRead(arrSensor[4]) > threshold);
    sensors[36].line = (analogRead(arrSensor[5]) > threshold);
  
  setMultiplexerInput(3);  
    sensors[4].line  = (analogRead(arrSensor[0]) > threshold);
    sensors[20].line = (analogRead(arrSensor[1]) > threshold);
    sensors[42].line = (analogRead(arrSensor[2]) > threshold);
    sensors[12].line = (analogRead(arrSensor[3]) > threshold);
    sensors[26].line = (analogRead(arrSensor[4]) > threshold);
    sensors[37].line = (analogRead(arrSensor[5]) > threshold);
    
  setMultiplexerInput(4);  
    sensors[5].line  = (analogRead(arrSensor[0]) > threshold);
    sensors[23].line = (analogRead(arrSensor[1]) > threshold);
    sensors[43].line = (analogRead(arrSensor[2]) > threshold);
    sensors[13].line = (analogRead(arrSensor[3]) > threshold);
    sensors[27].line = (analogRead(arrSensor[4]) > threshold);
    sensors[41].line = (analogRead(arrSensor[5]) > threshold);
  
  setMultiplexerInput(5);  
    sensors[6].line  = (analogRead(arrSensor[0]) > threshold);
    sensors[28].line = (analogRead(arrSensor[1]) > threshold);
    sensors[44].line = (analogRead(arrSensor[2]) > threshold);
    sensors[15].line = (analogRead(arrSensor[3]) > threshold);
    sensors[29].line = (analogRead(arrSensor[4]) > threshold);
    sensors[45].line = (analogRead(arrSensor[5]) > threshold);
    
  setMultiplexerInput(6);  
    sensors[8].line  = (analogRead(arrSensor[0]) > threshold);
    sensors[30].line = (analogRead(arrSensor[1]) > threshold);
    sensors[46].line = (analogRead(arrSensor[2]) > threshold);
    sensors[9].line  = (analogRead(arrSensor[3]) > threshold);
    sensors[24].line = (analogRead(arrSensor[4]) > threshold);
    sensors[39].line = (analogRead(arrSensor[5]) > threshold);
  
  setMultiplexerInput(7);  
    sensors[14].line = (analogRead(arrSensor[0]) > threshold);
    sensors[31].line = (analogRead(arrSensor[1]) > threshold);
    sensors[47].line = (analogRead(arrSensor[2]) > threshold);
    sensors[10].line = (analogRead(arrSensor[3]) > threshold);
    sensors[25].line = (analogRead(arrSensor[4]) > threshold);
    sensors[40].line = (analogRead(arrSensor[5]) > threshold);
  
  for(int i = 0; i < numOfSensors; i++) {
    if(sensors[i].line) {
      lineSeen = true; 
      break;
    }
  }
  
}

byte encodeValue(double value, int maxValue) {
  if(maxValue <= 0) return 127;
  double normalized = (value + maxValue) / (2.0 * maxValue);
  if(normalized < 0) normalized = 0;
  if(normalized > 1) normalized = 1;
  return (byte)(normalized * 255.0);
}

void sendCoords(Vector v, bool lineSeen, bool lineJumped, int radius) {
  // IDEE: eventuell braucht man keinen Vektor erstellen, sondern "bearbeitet" die Koordinaten (stretch + rotate)
  byte first_package = 0;
  if(lineSeen) first_package = first_package + 64;
  if(lineJumped) first_package = first_package + 32;
  package[0] = first_package;
  package[1] = encodeValue(v.getX(), radius);
  package[2] = encodeValue(v.getY(), radius);
}

bool isLineJumped(Vector vCurrent, Vector vFirst) {
  double offset = Vector::angleOffsetBetween(vCurrent, vFirst);
  bool isJumped = (ABS(offset) > (0.5 * PI));
  return isJumped;
}

void DEBUG_PRINT(bool lineSeen, bool lineJumped, int cloudCount, Vector v, double rad, int radius) {
  Serial.println("############ DEBUG START ############");
  
  //Serial.println("lineSeen: " + String(lineSeen));
  Serial.println("lineJumped: " + String(lineJumped));
  //Serial.println("radius: " + String(radius));
  Serial.println("cloudCount: " + String(cloudCount));
  //Serial.println("rad: " + String(rad));

  Serial.print("arr Sensors:");
  for(int j = 0; j < numOfSensors; j++) {
    Serial.print(String(sensors[j].line) + ", ");
  } Serial.println("");
  

  v.debug_println();
  //Serial.println("dir: " + String(vecToDir(v)));
  //Serial.println("");

  Serial.println("############  DEBUG END  ############");
  Serial.println("");
}

void recognizeLine() {
  lineSeen = false;
  sensorRead();
  
  if(!lineSeen) {
    while (Serial.available() > 0) {
        Serial.read();
    }
    vFirstLine = Vector();
    sendCoords(vFirstLine, lineSeen, lineJumped, -1);
    lineJumped = false;
    countJumpedLine = 0;
    return; 
  }
  
  Cloud clouds[MAX_CLOUDS];
  int cloudCount;
  findClouds(clouds, cloudCount);

  double midpoints[MAX_CLOUDS];
  MidpointInRad(clouds, cloudCount, midpoints); // start und end werden in rad umgerechnet und dann wird der Mittelpunkt jeder Cloud bestimmt

  double rad = averageRad(midpoints, cloudCount);

  Vector vLine = Vector(1, 0);
  vLine.rotate(rad);

  if(vFirstLine.getRad() < 0.001) {
    vFirstLine = vLine;
  }

  //lineJumped
  lineJumped = isLineJumped(vLine, vFirstLine);

  // Mittelwert von allen sensor[...].radius berechnen
  int midRadius = calculateMidRadius();
  if(midRadius != 0) vLine.stretch(midRadius);
  sendCoords(vLine, lineSeen, lineJumped, midRadius);

  // DEBUG
  bool debug = Serial.available() > 0;
  while (Serial.available() > 0) {
    Serial.read();  // Buffer leeren
  }

  if(debug) DEBUG_PRINT(lineSeen, lineJumped, cloudCount, vLine, rad, midRadius);
}

void loop() {
  digitalWrite(led_addr, HIGH);
  delayMicroseconds(100);
  recognizeLine();
}