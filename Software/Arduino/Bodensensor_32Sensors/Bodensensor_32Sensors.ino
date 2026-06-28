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

void ERROR_BLINK(int time, int wait) { // time = milliseconds
  for(int i = 0; i > int(time / wait); i++) {
    digitalWrite(led_addr, HIGH);
    delayMicroseconds(wait);
    digitalWrite(led_addr, LOW);
    delayMicroseconds(wait);
  }
  digitalWrite(led_addr, HIGH);
}

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

double numInRad(int num) {
  const double FACTOR = 11.25 * (M_PI / 180.0); 
  return FACTOR * num;
}

void MidpointInRad(const Cloud clouds[], int cloudCount, double midpoints[]) {
  for(int i = 0; i < cloudCount; i++) {
    double startRad = numInRad(clouds[i].start);
    double endRad = numInRad(clouds[i].end);
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

void areCloudsConnected(int arr[], Cloud clouds[], int &cloudCount) {
  if (cloudCount < 2) return; // nur eine Cloud, nichts zu prüfen

  int zerosBetween = 0;
  int i = (clouds[cloudCount - 1].end + 1) % numOfSensors; // die stelle wo die letzte cloud geendet ist + 1

  while (i != clouds[0].start) {
      if (arr[i] == 0) {
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

void findClouds(int arr[], Cloud clouds[], int &cloudCount) {
  cloudCount = 0;
  int start = -1;          // -1 = keine Cloud aktiv
  int zeros_in_a_row = 0;

  for (int i = 0; i < numOfSensors; i++) {
      if (arr[i] == 1) {
          if (start == -1) start = i; // neue Cloud beginnt
          zeros_in_a_row = 0;
      } 
      else { // arr[i] == 0
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

  areCloudsConnected(arr, clouds, cloudCount);
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

int* sensorRead() {
  /*
  save 1 for Line seen and 0 for not seen
  OUTPUT: arr -> array of 32 for each sensor if line was detected or not
  */
  
  static int arr[numOfSensors];

  lineSeen = false;
  // die Richtungen sind von oben betrachtet -> auf den bot von oben schauen
  setMultiplexerInput(2);  // r3
    arr[3] = (analogRead(sensor[0]) > threshold) ? 1 : 0; // 1 = Linie wurde erkannt, 0 = Linie wurde nicht erkannt
    arr[11] = (analogRead(sensor[1]) > threshold) ? 1 : 0;
    arr[19] = (analogRead(sensor[2]) > threshold) ? 1 : 0;
    arr[27] = (analogRead(sensor[3]) > threshold) ? 1 : 0;
    
  setMultiplexerInput(4);  //r2
    arr[2] = (analogRead(sensor[0]) > threshold) ? 1 : 0;
    arr[10] = (analogRead(sensor[1]) > threshold) ? 1 : 0;
    arr[18] = (analogRead(sensor[2]) > threshold) ? 1 : 0;
    arr[26] = (analogRead(sensor[3]) > threshold) ? 1 : 0;
    
  setMultiplexerInput(0);  //r1
    arr[1] = (analogRead(sensor[0]) > threshold) ? 1 : 0;
    arr[9] = (analogRead(sensor[1]) > threshold) ? 1 : 0;
    arr[17] = (analogRead(sensor[2]) > threshold) ? 1 : 0;
    arr[25] = (analogRead(sensor[3]) > threshold) ? 1 : 0;
  
  setMultiplexerInput(6);  //mitte
    arr[0] = (analogRead(sensor[0]) > threshold) ? 1 : 0;
    arr[8] = (analogRead(sensor[1]) > threshold) ? 1 : 0;
    arr[16] = (analogRead(sensor[2]) > threshold) ? 1 : 0;
    arr[24] = (analogRead(sensor[3]) > threshold) ? 1 : 0;
    
  setMultiplexerInput(5);  //l1
    arr[31] = (analogRead(sensor[0]) > threshold) ? 1 : 0;
    arr[7] = (analogRead(sensor[1]) > threshold) ? 1 : 0;
    arr[15] = (analogRead(sensor[2]) > threshold) ? 1 : 0;
    arr[23] = (analogRead(sensor[3]) > threshold) ? 1 : 0;
  
  setMultiplexerInput(7);  //l2
    arr[30] = (analogRead(sensor[0]) > threshold) ? 1 : 0;
    arr[6] = (analogRead(sensor[1]) > threshold) ? 1 : 0;
    arr[14] = (analogRead(sensor[2]) > threshold) ? 1 : 0;
    arr[22] = (analogRead(sensor[3]) > threshold) ? 1 : 0;
    
  setMultiplexerInput(3);  //l3
    arr[29] = (analogRead(sensor[0]) > threshold) ? 1 : 0;
    arr[5] = (analogRead(sensor[1]) > threshold) ? 1 : 0;
    arr[13] = (analogRead(sensor[2]) > threshold) ? 1 : 0;
    arr[21] = (analogRead(sensor[3]) > threshold) ? 1 : 0;
  
  setMultiplexerInput(1);  //l4
    arr[28] = (analogRead(sensor[0]) > threshold) ? 1 : 0;
    arr[4] = (analogRead(sensor[1]) > threshold) ? 1 : 0;
    arr[12] = (analogRead(sensor[2]) > threshold) ? 1 : 0;
    arr[20] = (analogRead(sensor[3]) > threshold) ? 1 : 0;
  
  for(int i = 0; i < numOfSensors; i++) {
    if(arr[i] == 1) {
      lineSeen = true; 
      break;
    }
  }
  
  return arr;
}

byte encodeValue(double value) {
  double normalized = (value + radius) / (2.0 * radius);
  if(normalized < 0) normalized = 0;
  if(normalized > 1) normalized = 1;
  return (byte)(normalized * 255.0);
}

void sendCoords(Vector v, bool lineSeen, bool lineJumped) {
  // IDEE: eventuell braucht man keinen Vektor erstellen, sondern "bearbeitet" die Koordinaten (stretch + rotate)
  byte first_package = 0;
  if(lineSeen) first_package = first_package + 64;
  if(lineJumped) first_package = first_package + 32;
  package[0] = first_package;
  package[1] = encodeValue(v.getX());
  package[2] = encodeValue(v.getY());

}

void DEBUG_PRINT(bool lineSeen, bool lineJumped, int arr[], int cloudCount, Vector v) {
  Serial.println("############ DEBUG START ############");
  
  Serial.println("lineSeen: " + String(lineSeen));
  Serial.println("lineJumped: " + String(lineJumped));
  Serial.println("cloudCount: " + String(cloudCount));

  Serial.print("arr:");
  for(int j = 0; j < numOfSensors; j++) {
    Serial.print(String(arr[j]) + ", ");
  } Serial.println("");

  v.debug_println();
  Serial.println("dir: " + String(vecToDir(v)));
  Serial.println("");

  Serial.println("############  DEBUG END  ############");
  Serial.println("");
}

void recognizeLine() {
  // TODO: nur weiter gehen nach sensorRead() wenn wirklich etwas gefunden wurde
  lineSeen = false;
  //lineJumped = false;
  int* arrLineSeen = sensorRead();
  if(!lineSeen) {
    sendCoords(Vector(), lineSeen, lineJumped);
    lineJumped = false;
    return; 
  }
  Cloud clouds[MAX_CLOUDS];
  int cloudCount;
  findClouds(arrLineSeen, clouds, cloudCount);

  double midpoints[MAX_CLOUDS];
  MidpointInRad(clouds, cloudCount, midpoints); // start und end werden in rad umgerechnet und dann wird der Mittelpunkt jeder Cloud bestimmt

  double rad = averageRad(midpoints, cloudCount);

  // LineJumped
  if(lastRad <= double(2.0*PI) && ABS(ABS(lastRad) - ABS(rad)) > minAngleJumped) {
      lineJumped = true;
      countJumpedLine++;
    }
    if(lineJumped && (countJumpedLine >= 2 || !lineSeen)) {
      lineJumped = false;
      countJumpedLine = 0;
    }
    lastRad = rad;
  
  Vector vLine = Vector(1, 0);
  vLine.rotate(rad);
  vLine.stretch(radius);
  sendCoords(vLine, lineSeen, lineJumped);

  // DEBUG
  bool debug = Serial.available() > 0;
  while (Serial.available() > 0) {
    Serial.read();  // Buffer leeren
  }

  if(debug) DEBUG_PRINT(lineSeen, lineJumped, arrLineSeen, cloudCount, vLine);

}

void loop() {
  digitalWrite(led_addr, HIGH);
  delayMicroseconds(100);
  recognizeLine();
}
