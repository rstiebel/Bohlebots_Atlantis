
struct AngleInterval {
  double from;
  double to;
};


double OffsetAngleObject(Vector v, int width) {
  if(v.getRad() == 0) return 0.0;
  double angle = atan((width/2.0) / v.getRad());
  return angle;
}

double OffsetAngleGoal(Vector v, int width) {
  if(v.getRad() == 0) return 0.0;
  Vector vOffsetGoal = Vector::rotate((vMidAxis.unit() * (width / 2.0)), 0.5*PI);
  vOffsetGoal = v + vOffsetGoal;
  double angle = Vector::angleOffsetBetween(v, vOffsetGoal);
  return angle;
}

/*
bool isWayBlocked(Vector v, int width) {
  double offsetTarget = OffsetAngleGoal(v, width);
  Vector vMidPointObject;
  bool blocked = false;
  if(!enemySeen) return false;
  for(int i = 0; i < MAX_NUM_OBJECTS; i++) {
    if(!enemies[i].onField) continue;
    vMidPointObject = enemies[i].pos;
    double offsetObject = OffsetAngleObject(vMidPointObject, OBJECT_WIDTH);
    offsetObject += 0.05*PI; // 9 grad
    int side = vMidPointObject.sideOf(v);
    vMidPointObject.rotate(offsetObject*(-side));
    Vector vOffsetGoal = Vector::rotate((vMidAxis.unit() * (width / 2.0)), 0.5*PI*(-side));
    vOffsetGoal = v + vOffsetGoal;
    blocked = blocked || (Vector::angleOffsetBetween(vOffsetGoal, vMidPointObject) <= 0.05*PI);
  }
  return blocked;
}
*/

int setInterval(Vector v, int width, AngleInterval& IntervalTarget, AngleInterval shadows[], uint numShadows) {
  double angleOffsetTarget = OffsetAngleGoal(v, width);
  IntervalTarget.from = -angleOffsetTarget;
  IntervalTarget.to = angleOffsetTarget;
  
  int count = 0;
  for (int i = 0; i < MAX_NUM_OBJECTS; i++) {
    if (!enemies[i].onField) continue;
    Vector vMidPointObject = enemies[i].pos;
    double angleOffsetObject = OffsetAngleObject(vMidPointObject, OBJECT_WIDTH);

    double shift = vMidPointObject.getAlpha() - v.getAlpha();
    if (shift > PI) shift -= 2 * PI;
    if (shift < -PI) shift += 2 * PI;

    double from = shift - angleOffsetObject;
    double to   = shift + angleOffsetObject;

    if (to < IntervalTarget.from || from > IntervalTarget.to) continue; // schauen ob roboter im targetInterval ist

    if (from < IntervalTarget.from) from = IntervalTarget.from;
    if (to > IntervalTarget.to) to = IntervalTarget.to;

    shadows[count].from = from;
    shadows[count].to = to;
    count++;
  }

  // nach "from" sortieren
  for (int i = 1; i < MIN(count, numShadows); i++) {
    AngleInterval current = shadows[i];
    int j = i - 1;
    while ((j >= 0) && (shadows[j].from > current.from)) {
      shadows[j + 1] = shadows[j];
      j--;
    }
    shadows[j + 1] = current;
  }

  return count;
}

bool findWidestGap(AngleInterval IntervalTarget, AngleInterval shadows[], int count, AngleInterval &gap) {
  double covered = IntervalTarget.from;
  double bestWidth = (IntervalTarget.to - IntervalTarget.from) / 5.0;
  bool found = false;

  // Setzt gap auf die größte Lücke im Intervall falls vorhanden
  for (int i = 0; i < count; i++) {
    if (shadows[i].from > covered) {
      double gapWidth = shadows[i].from - covered;
      if (gapWidth > bestWidth) {
        bestWidth = gapWidth; 
        gap.from = covered;
        gap.to = shadows[i].from;
        found = true;
      }
    }
    if (shadows[i].to > covered) covered = shadows[i].to;
  }

  // Kontrolliert ob letzte Lücke größte ist
  if (IntervalTarget.to - covered > bestWidth) {
    bestWidth = IntervalTarget.to - covered;
    if (bestWidth > 0) {
      gap.from = covered;
      gap.to = IntervalTarget.to;
      found = true;
    }
  }

  return found;
}

Vector setGapVector(Vector v, AngleInterval gap) {
  double gapMidAngle = (gap.from + gap.to) / 2.0;

  return Vector::rotate(v, gapMidAngle);
}

Vector findGap(Vector v, int width) {
  if (!enemySeen) return v; // niemand im Weg -> direkt aufs Ziel zielen

  AngleInterval IntervalTarget;
  AngleInterval shadows[MAX_NUM_OBJECTS];

  int count = setInterval(v, width, IntervalTarget, shadows, MAX_NUM_OBJECTS);

  AngleInterval gap;
  bool found = findWidestGap(IntervalTarget, shadows, count, gap);

  if (!found) return Vector(); // komplett blockiert -> hier müsstest du entscheiden, was dann passieren soll

  return setGapVector(v, gap);
}

void TestFlank() {
  Vector vTarget;
  vTarget = findGap(vGoal, GOAL_WIDTH);
  drive(0, 0, selectRotation(ROTATION_TARGET_OBJECT, vTarget));
}

Vector findNearestEnemyToOwnGoal() {
  if(!enemySeen) return Vector();
  Vector v;
  Vector vNearestEnemy;
  bool found = false;
  int minDist = 255;
  for(int i = 0; i < MAX_NUM_OBJECTS; i++) {
    if(!enemies[i].onField) continue;
    v = enemies[i].pos;
    if(!found || enemies[i].relativPosToOwnGoal.getRad() < minDist) {
      vNearestEnemy = v;
      minDist = enemies[i].relativPosToOwnGoal.getRad();
      found = true;
    }
  }
  return vNearestEnemy;
}

Vector findNearestEnemy() {
  if(!enemySeen) return Vector();
  Vector v;
  Vector vNearestEnemy;
  bool found = false;
  for(int i = 0; i < MAX_NUM_OBJECTS; i++) {
    if(!enemies[i].onField) continue;
    v = enemies[i].pos;
    if(!found || v.getRad() < vNearestEnemy.getRad()) {
      vNearestEnemy = v;
      found = true;
    }
  }
  return vNearestEnemy;
}

Vector findMovingEnemy() {
  if(!enemySeen) return Vector();
  Vector v;
  Vector vTargetEnemy;
  bool found = false;
  for(int i = 0; i < MAX_NUM_OBJECTS; i++) {
    if(!enemies[i].onField) continue;
    v = enemies[i].pos;
    Vector vVelo = correctedVector(enemies[i].velocity);
    bool isMovingTowardsGoal = (ABS(vVelo.getAlpha()) > 0.5*PI);
    if(isMovingTowardsGoal != enemies[i].movingTowardsGoal) {
      enemies[i].velocityStabelCount++;
    }
    if(enemies[i].velocityStabelCount >= 3) {
      enemies[i].movingTowardsGoal = isMovingTowardsGoal;
      enemies[i].velocityStabelCount = 0;
    }
    bool isNearest = (!found || (v.getRad() < vTargetEnemy.getRad()));
    if(enemies[i].movingTowardsGoal && isNearest) {
      vTargetEnemy = v;
      found = true;
    }
  }
  return vTargetEnemy;
}