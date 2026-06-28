#define BALL_ROT_MAX_DIST 20

int rotToMidAxis()
{
  int r = 0;
  if(midPointValid) {
    r = vMidAxis.getAlpha() * 10; // turn towards MidAxis
  } else if(goalSeen) {
    r = vGoal.getAlpha() * 10; // turn towards Goal
  } else if(ownGoalSeen) {
    r = Vector::rotate(vOwnGoal, PI).getAlpha() * 10; // turn away from OwnGoal
  }
  return r;
}

int rotToMidAxisSoft()
{
  int r = 0;
  if(midPointValid) {
    r = vMidAxis.getAlpha() * 10; // turn towards MidAxis
  } else if(goalSeen) {
    r = vGoal.getAlpha() * 10; // turn towards Goal
  } else if(ownGoalSeen) {
    r = Vector::rotate(vOwnGoal, PI).getAlpha() * 10; // turn away from OwnGoal
  }
  // funktion die kleinere werte nach oben zieht, aber trotzdem null werden kann 
  if (ABS(r) <= 2) return r;
  int sign = (r > 0) ? 1 : -1;
  return sign * MAX(ABS(r), 8);
}


int rotToInvertedMidAxis()
{
  Vector vInvertedMidAxis = Vector::rotate(vMidAxis, PI);
  int r = 0;
  if(midPointValid) {
    r = vInvertedMidAxis.getAlpha() * 10; // turn towards MidAxis
  } else if(goalSeen) {
    r = vGoal.getAlpha() * 10; // turn towards Goal
  } else if(ownGoalSeen) {
    r = Vector::rotate(vOwnGoal, PI).getAlpha() * 10; // turn away from OwnGoal
  }

  // funktion die kleinere werte nach oben zieht, aber trotzdem null werden kann 
  if (ABS(r) <= 2) return r;
  int sign = (r > 0) ? 1 : -1;
  return sign * MAX(ABS(r), 8);
}


int rotToScore()
{
  int r = 0;
  if(goalSeen) {
    r = vGoal.getAlpha() * 10; // turn towards Goal
  } else if(ownGoalSeen) {
    r = Vector::rotate(vOwnGoal, PI).getAlpha() * 10; // turn away from OwnGoal
  }
  if(midPointValid && (ABS(vGoal.getAlpha()) > 0.5*PI)) {
    Vector vMidPointToBot = vMidPoint * -1.0;
    int side = vMidAxis.sideOf(vMidPointToBot); // side of the robot in the field, independent of its rotation
    r = -side*MAX_ROTATION;
  }
  return r;
}

int rotToInvertedScore() {
  int r = 0;
  if(goalSeen) {
    r = Vector::rotate(vGoal, PI).getAlpha() * 20; // turn away from Goal
  } else if(ownGoalSeen) {
    r = vOwnGoal.getAlpha() * 10; // turn to OwnGoal
  }
  return r;
}

int rotToOwnGoal() {
  int r = 0;
  if(ownGoalSeen) {
    r = Vector::rotate(vOwnGoal, PI).getAlpha() * 10; // turn towards Goal
  } 
  else {
    r = rotToMidAxis();
  }
  return r;
}

double normalizeAngle(double a) {
    while (a >  PI) a -= 2.0 * PI;
    while (a < -PI) a += 2.0 * PI;
    return a;
}

int rotToBall() {
  if(!ballSeen || !midPointValid || ABS(vMidAxis.getAlpha()) > 0.2*PI) {
      return rotToMidAxis();
  }
  return vBall.getAlpha() * 10;
}


int rotToBallMidAxis() {
  if(!midPointValid || !ballSeen || vBall.getRad() > BALL_ROT_MAX_DIST || ABS(vBall.getAlpha()) > 0.25*PI) {
    return rotToMidAxis();
  }

  static float lastRotAlpha = 0;

  float rawBallDistNorm = BOUND(vBall.getRad() / BALL_ROT_MAX_DIST, 0.8, 1.0); 
  const float minInput  = 0.8f;
  const float maxInput  = 1.0f;
  const float minOutput = 0.0f;
  const float maxOutput = 0.4f;

  float weightMidAxis = minOutput + ((maxOutput - minOutput) / (maxInput - minInput)) * (rawBallDistNorm - minInput);
  float weightBall = 1.0f - weightMidAxis;

  Vector vMidAxisUnit = vMidAxis.unit();
  Vector vBallUnit = vBall.unit();
  Vector vRotation = (vBallUnit * weightBall) + (vMidAxisUnit * weightMidAxis);

  float dAlpha = vRotation.getAlpha() - vMidAxis.getAlpha();
  while (dAlpha >  PI) dAlpha -= 2.0f * PI;
  while (dAlpha < -PI) dAlpha += 2.0f * PI;

  if (dAlpha > +0.3*PI) vRotation.rotate(-dAlpha + 0.3*PI);
  if (dAlpha < -0.3*PI) vRotation.rotate(-dAlpha - 0.3*PI);

  float currentAlpha = vRotation.getAlpha();
  float d = (currentAlpha - lastRotAlpha) * 6.0f;  // D-Faktor anpassen
  lastRotAlpha = currentAlpha;

  return (int)(currentAlpha * 10 - d);
}

int rotToObject(Vector v) {
  return v.getAlpha() * 10;
}

int rotToWall() {
  if(!midPointValid) {
    return rotToMidAxis();
  }

  double destinatedAngle = (vMidPoint.getAlpha() < 0.0 ? 0.15*PI : -0.15*PI);
  double delta = destinatedAngle - vMidAxis.getAlpha();

  while (delta >  PI) delta -= 2.0 * PI;
  while (delta < -PI) delta += 2.0 * PI;

  Serial.println("destinatedAngle: " + String(destinatedAngle));
  Serial.println("vMidAxis Alpha: " + String(vMidAxis.getAlpha()));
  vMidPoint.debug_println();
  vMidAxis.debug_println();
  Serial.println("delta: " + String(delta));

  return 0; // (int)(delta * 5);
}

int rotToSide() {
  Vector v = vGoal - vMidPoint;
  int side = (vMidPointCorrected.getAlpha() < 0 ? 1 : -1);
  v.rotate(0.5*PI*side);
  return v.getAlpha() * 10;
}

enum ROTATION_TARGET {
  ROTATION_TARGET_MIDAXIS,
  ROTATION_TARGET_BALL,
  ROTATION_TARGET_SCORE,
  ROTATION_TARGET_SCORE_INVERTED,
  ROTATION_TARGET_OWNGOAL,
  ROTATION_TARGET_MIDAXIS_BALL,
  ROTATION_TARGET_INVERTED_MIDAXIS,
  ROTATION_TARGET_MIDAXIS_SOFT,
  ROTATION_TARGET_OBJECT,
  ROTATION_TARGET_SIDE,
  ROTATION_TARGET_WALL,
  ROTATION_TARGET_NONE
};


int selectRotation(ROTATION_TARGET rotationTarget, Vector v = Vector()) {
  int r = 0;
  switch(rotationTarget) {
    case ROTATION_TARGET_MIDAXIS: r = rotToMidAxis(); break;
    case ROTATION_TARGET_BALL: r = rotToBall(); break;
    case ROTATION_TARGET_SCORE: r = rotToScore(); break;
    case ROTATION_TARGET_SCORE_INVERTED: r = rotToInvertedScore(); break;
    case ROTATION_TARGET_OWNGOAL: r = rotToOwnGoal(); break;
    case ROTATION_TARGET_MIDAXIS_BALL: r = rotToBallMidAxis(); break;
    case ROTATION_TARGET_INVERTED_MIDAXIS: r = rotToInvertedMidAxis(); break;
    case ROTATION_TARGET_MIDAXIS_SOFT: r = rotToMidAxisSoft(); break;
    case ROTATION_TARGET_OBJECT: r = rotToObject(v); break;
    case ROTATION_TARGET_SIDE: r = rotToSide(); break;
    case ROTATION_TARGET_WALL: r = rotToWall(); break;
    case ROTATION_TARGET_NONE: r = 0; break;
    default: break;
  }
  return r;
}
