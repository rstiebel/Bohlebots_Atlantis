double calculateAngle(int n) {
  return n / 255.0 * (2*PI) - PI;
}
void readPositions() {
  // Ball
  for (int i = 0; i < MAX_NUM_BALL; i++) {
    while (Serial2.available() < 2); // warten bis 2 Bytes da
    ball[i][0] = Serial2.read(); // angle
    ball[i][1] = Serial2.read(); // dist
  }

  // Yellow
  for (int i = 0; i < MAX_NUM_YELLOW; i++) {
    while (Serial2.available() < 2);
    yellow[i][0] = Serial2.read();
    yellow[i][1] = Serial2.read();
  }

  // Blue
  for (int i = 0; i < MAX_NUM_BLUE; i++) {
    while (Serial2.available() < 2);
    blue[i][0] = Serial2.read();
    blue[i][1] = Serial2.read();
  }

  // Lines (8 Bytes)
  for (int i = 0; i < MAX_NUM_LINES; i++) {
    while (Serial2.available() < 1);
    lines[i] = Serial2.read();
  }

  // Objects (maximal MAX_OBJECTS)
  for (int i = 0; i < MAX_NUM_OBJECTS; i++) {
    while (Serial2.available() < 2);
    objects[i][0] = Serial2.read();
    objects[i][1] = Serial2.read();
  }

  if(yellow[0][1] == 255) {
      vYellow = Vector();  // leerer Vektor, wenn Distanz ungültig
  } 
  else {
      double yellowAngle = calculateAngle(yellow[0][0]);
      vYellow = Vector(1, 0);
      vYellow.rotate(yellowAngle);
      vYellow.stretch(yellow[0][1]);
  }

  if(blue[0][1] == 255) {
      vBlue = Vector();  // leerer Vektor, wenn Distanz ungültig
  } 
  else {
      double blueAngle = calculateAngle(blue[0][0]);
      vBlue = Vector(1, 0);
      vBlue.rotate(blueAngle);
      vBlue.stretch(blue[0][1]);
  }

  if(ball[0][1] == 255) {
      vBall = Vector();  // leerer Vektor, wenn Distanz ungültig
  } 
  else {
      double ballAngle = calculateAngle(ball[0][0]);
      vBall = Vector(1, 0);
      vBall.rotate(ballAngle);
      vBall.stretch(ball[0][1]);
  }

}

void readData() {
  if (Serial2.available() >= 2) {
    if (Serial2.peek() == START_BYTE_1) {
      Serial2.read(); 
      if (Serial2.peek() == START_BYTE_2) {
        Serial2.read(); 

        readPositions();
      }
    } 
    else {
      Serial2.read(); // falsches Byte verwerfen
    }
  }
}