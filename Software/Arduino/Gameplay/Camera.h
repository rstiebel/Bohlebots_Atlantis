double calculateAngle(int n) {
  return n / 255.0 * (2 * PI) - PI;
}

Vector correctedVector(Vector v) {
  if(midPointValid) {
    Vector correctedV = Vector::rotate(v, -vMidAxis.getAlpha());
    return correctedV;
  }
  return v;
}

void fillCostMatrix(int numOfObjects) {
  for(int i = 0; i < MAX_NUM_OBJECTS; i++) {
    Vector vPredicted = enemies[i].pos + enemies[i].velocity;
    for(int j = 0; j < MAX_NUM_OBJECTS; j++) {
      if (!enemies[i].onField || j >= numOfObjects) {
        // beim letzten frame gab es an der stelle noch keinen Gegner oder ein Gegner existiert nicht
        cost[i][j] = 999999.0;
        continue;
      }
      Vector vNewObject = Vector(1, 0);
      double angleOfObject = calculateAngle(objects[j][0]);
      vNewObject.rotate(angleOfObject);
      vNewObject.stretch(objects[j][1]);
      cost[i][j] = (vPredicted - vNewObject).getRad();
    }
  }
}

int findBestPerm() {
  double bestCost = 999999.0;
  int bestPerm = 0;

  for (int p = 0; p < NUM_OF_PERMS; p++) {
      double totalCost = 0;
      for (int i = 0; i < MAX_NUM_OBJECTS; i++) {
          int j = combinations[p][i];  // welches Objekt bekommt Gegner i?
          totalCost += cost[i][j];
      }
      if (totalCost < bestCost) {
          bestCost = totalCost;
          bestPerm = p;
      }
  }
  return bestPerm;
}

void updateEnemies(int bestPerm, int numOfObjects) {
  bool usedObject[MAX_NUM_OBJECTS] = {false};

  for (int i = 0; i < MAX_NUM_OBJECTS; i++) {
    if (!enemies[i].onField) continue;
    int j = combinations[bestPerm][i];

    if (cost[i][j] >= 999999.0) {
      // Gegner hat kein Objekt bekommen -> verschwunden
      enemies[i].onField                  = false;
      enemies[i].velocity                 = Vector();
      enemies[i].pos                      = Vector();
      enemies[i].relativPosToOwnGoal      = Vector();
      continue;
    }

    Vector vNewEnemy = Vector(1, 0);
    vNewEnemy.rotate(calculateAngle(objects[j][0]));
    vNewEnemy.stretch(objects[j][1]);

    Vector vOwnGoalToEnemy = vNewEnemy - vOwnGoal;
    
    enemies[i].velocity            = vOwnGoalToEnemy - enemies[i].relativPosToOwnGoal;
    enemies[i].pos                 = vNewEnemy;
    enemies[i].relativPosToOwnGoal = vOwnGoalToEnemy;
    usedObject[j]                  = true;
  }

  // nicht zugeordnete Objekte -> neue Gegner
  for (int j = 0; j < numOfObjects; j++) {
    if (usedObject[j]) continue;
    for (int i = 0; i < MAX_NUM_OBJECTS; i++) {
      if (!enemies[i].onField) {
        Vector v = Vector(1, 0);
        v.rotate(calculateAngle(objects[j][0]));
        v.stretch(objects[j][1]);
        Vector vOwnGoalToEnemy = v - vOwnGoal;
        enemies[i].pos      = v;
        enemies[i].velocity = Vector();
        enemies[i].relativPosToOwnGoal = vOwnGoalToEnemy;
        enemies[i].onField  = true;
        break;
      }
    }
  }
}

void manageEnemies(int numOfObjects) {
  fillCostMatrix(numOfObjects);
  /*
  cost Tabelle ist erstellt 
  -> die Zeilen sind die predictions 
  -> die spalten sind die neuen Positionen die reinkommen
  */

  int bestPerm = findBestPerm();
  /*
  die beste Kombination ist bestPerm
  */
  updateEnemies(bestPerm, numOfObjects);
}

void readPositions(byte* buf, int numOfObjects) {
  int idx = 0;

  for (int i = 0; i < MAX_NUM_BALL;   i++) { ball[i][0]    = buf[idx]; idx++; ball[i][1]    = buf[idx]; idx++; }
  for (int i = 0; i < MAX_NUM_YELLOW; i++) { yellow[i][0]  = buf[idx]; idx++; yellow[i][1]  = buf[idx]; idx++; }
  for (int i = 0; i < MAX_NUM_BLUE;   i++) { blue[i][0]    = buf[idx]; idx++; blue[i][1]    = buf[idx]; idx++; }
  for (int i = 0; i < numOfObjects;   i++) { objects[i][0] = buf[idx]; idx++; objects[i][1] = buf[idx]; idx++; }

  if(numOfObjects > 0) {
    manageEnemies(numOfObjects);
    enemySeen = true;
  }
  else enemySeen = false;

  // -------- Yellow Vector --------
  if (yellow[0][1] == 255) {
    if(eepromData.scoreOnYellow) vGoal = Vector();
    else vOwnGoal = Vector();
    yellowSeen = false;
  } 
  else {
    double yellowAngle = calculateAngle(yellow[0][0]);
    yellowSeen = true;
    if(eepromData.scoreOnYellow) {
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
    if(!eepromData.scoreOnYellow) vGoal = Vector();
    else vOwnGoal = Vector();
    blueSeen = false;
  } 
  else {
    double blueAngle = calculateAngle(blue[0][0]);
    blueSeen = true;
    if(!eepromData.scoreOnYellow) {
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
    vMidPointCorrected = Vector::rotate(vMidPoint, -vMidAxis.getAlpha());
    vMidAxis  = (vGoal - vOwnGoal);
  } else {
    vMidPoint = Vector();
    vMidPointCorrected = Vector();
    vMidAxis  = Vector();
  }

  // -------- Ball Vector --------
  if (ball[0][1] == 255) {
      vBall = Vector();
      vLastBall = Vector();
      vBallVelocity = Vector();
      ballSeen = false;
  } else {
      double ballAngle = calculateAngle(ball[0][0]);
      vBall = Vector(1, 0);
      vBall.rotate(ballAngle);
      vBall.stretch(ball[0][1]);
      double emaBallVelo = 0.5;
      vBallVelocity = ((vBall - vLastBall) * emaBallVelo) + (vBallVelocity * (1.0 - emaBallVelo));
      vLastBall = vBall;
      ballSeen = true;


      Vector vBallToMidPoint = vMidPoint - vBall;
      sideOfBall = (vBallToMidPoint.getAlpha() < 0.0 ? 1 : -1);
  }
}

// ------------------------------------------------------
const int MAX_NUM_BYTES = 1 + ((MAX_NUM_BALL + MAX_NUM_YELLOW + MAX_NUM_BLUE + MAX_NUM_OBJECTS) * 2);  // NumObjects + Distance and Angle for each class
byte buffer[MAX_NUM_BYTES];

void readData() {
  while (Serial2.available() >= 2) {
    if (Serial2.peek() != START_BYTE_1) { Serial2.read(); continue; }
    Serial2.read();
    if (Serial2.peek() != START_BYTE_2) { continue; }
    Serial2.read();

    while (!Serial2.available()) { continue; }  // Wait for next byte to arrive
    byte numOfObjects = Serial2.read();
    if (numOfObjects > MAX_NUM_OBJECTS) numOfObjects = MAX_NUM_OBJECTS;

    int totalBytes = (MAX_NUM_BALL + MAX_NUM_YELLOW + MAX_NUM_BLUE + numOfObjects) * 2;
    memset(buffer, 0, MAX_NUM_BYTES); // buffer komplett mit 0 füllen
    if (Serial2.readBytes(buffer, totalBytes) < totalBytes) {
      while (Serial2.available()) Serial2.read();
      return;
    }

    // Buffer leeren - alte Frames wegwerfen
    while (Serial2.available()) Serial2.read();
    
    readPositions(buffer, numOfObjects);
    return;
  }
}