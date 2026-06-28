double calculateAngle(int n) {
  return n / 255.0 * (2 * PI) - PI;
}

void readPositions() {

  // -------- Anzahl Objects --------
  while (Serial2.available() < 1);
  byte numOfObjects = Serial2.read();

  if (numOfObjects > MAX_NUM_OBJECTS) {
    numOfObjects = MAX_NUM_OBJECTS;
  }

  // -------- Ball --------
  for (int i = 0; i < MAX_NUM_BALL; i++) {
    while (Serial2.available() < 2);
    ball[i][0] = Serial2.read();
    ball[i][1] = Serial2.read();
  }

  // -------- Yellow --------
  for (int i = 0; i < MAX_NUM_YELLOW; i++) {
    while (Serial2.available() < 2);
    yellow[i][0] = Serial2.read();
    yellow[i][1] = Serial2.read();
  }

  // -------- Blue --------
  for (int i = 0; i < MAX_NUM_BLUE; i++) {
    while (Serial2.available() < 2);
    blue[i][0] = Serial2.read();
    blue[i][1] = Serial2.read();
  }

  // -------- Lines --------
  for (int i = 0; i < MAX_NUM_LINES; i++) {
    while (Serial2.available() < 1);
    lines[i] = Serial2.read();
  }

  // -------- Objects (variabel) --------
  for (int i = 0; i < numOfObjects; i++) {
    while (Serial2.available() < 2);
    objects[i][0] = Serial2.read();
    objects[i][1] = Serial2.read();
  }

  // -------- Rest invalidieren --------
  for (int i = numOfObjects; i < MAX_NUM_OBJECTS; i++) {
    objects[i][0] = 0;
    objects[i][1] = 255;
  }

  // -------- Yellow Vector --------
  if (yellow[0][1] == 255) {
    if(scoreOnYellow) vGoal = Vector();
    else vOwnGoal = Vector();
    yellowSeen = false;
  } 
  else {
    double yellowAngle = calculateAngle(yellow[0][0]);
    yellowSeen = true;
    if(scoreOnYellow) {
      vGoal = Vector(1, 0);
      vGoal.rotate(yellowAngle);
      vGoal.stretch(yellow[0][1]);
    }
    else {
      vOwnGoal = Vector(1, 0);
      vOwnGoal.rotate(yellowAngle);
      vOwnGoal.stretch(yellow[0][1]);
    }
  }
  // -------- Blue Vector --------
  if (blue[0][1] == 255) {
    if(!scoreOnYellow) vGoal = Vector();
    else vOwnGoal = Vector();
    blueSeen = false;
  } 
  else {
    double blueAngle = calculateAngle(blue[0][0]);
    blueSeen = true;
    if(!scoreOnYellow) {
      vGoal = Vector(1, 0);
      vGoal.rotate(blueAngle);
      vGoal.stretch(blue[0][1]);
    }
    else {
      vOwnGoal = Vector(1, 0);
      vOwnGoal.rotate(blueAngle);
      vOwnGoal.stretch(blue[0][1]);
    }
  }

  // -------- Midpoint --------
  if (midPointValid) {
    vMidPoint = (vGoal + vOwnGoal) / 2;
    vMidAxis  = (vGoal - vOwnGoal);
  } else {
    vMidPoint = Vector();
    vMidAxis  = Vector();
  }
  vMidAxisAbsolute = vGoalAbsolutePos - vOwnGoalAbsolutePos;

  // Vector vBallNew = Vector()
  // -------- Ball Vector --------
  if (ball[0][1] == 255) {
    vBall = Vector();
    ballSeen = false;
  } else {
    double ballAngle = calculateAngle(ball[0][0]);
    vBall = Vector(1, 0);
    vBall.rotate(ballAngle);
    vBall.stretch(ball[0][1]);
    ballSeen = true;

    /*
    double ballAngle = calculateAngle(ball[0][0]);
    vBallNew = Vector(1, 0);
    vBallNew.rotate(ballAngle);
    vBallNew.stretch(ball[0][1]);
    ballSeen = true;


    double newBall_EMA = 1.0; // Exponential Moving Average
    vBall = (vBallNew * newBall_EMA) + (vBall * (1.0 - newBall_EMA));

    */
  }
}

// ------------------------------------------------------

void readData() {
  while (Serial2.available() >= 2) {
    if (Serial2.peek() == START_BYTE_1) {
      Serial2.read();
      if (Serial2.peek() == START_BYTE_2) {
        Serial2.read();
        readPositions();
        return;
      }
    }
    Serial2.read(); // Resync bei Müllbytes
  }
}
