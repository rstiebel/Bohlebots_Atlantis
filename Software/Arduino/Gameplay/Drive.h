
#define MAX_MIDPOINT_DIST 60.0 // TODO: could be different on another field

// TODO: could be different on another field
int computeMaxSpeed(Vector v, int maxSpd) {
  double speedFactor = 1.0;
  FieldPosition botPos = getPosOnField();

  if(!midPointValid) return 35;

  double angleBetweenGoals = Vector::angleOffsetBetween(vMidPointCorrected, v);
  if(angleBetweenGoals > 0.5*PI) {
    double MidPointRadNorm = BOUND(vMidPoint.getRad() / MAX_MIDPOINT_DIST, 0.0, 1.0); // -> je weiter weg vom Mittelpunkt desto größer der Wert
    // der Faktor sollte bei MidPointRadNorm nahe 1, größer werden 
    speedFactor = BOUND(1.0 - MidPointRadNorm, 0.55, 1.0);
  }

  if(botPos == IN_CORNER) speedFactor = 0.5;
  // noch der winkel zum tor 
  return maxSpd * speedFactor;
}
// TODO: could be different on another field
constexpr double pFactor = 2.0; // 1.8
constexpr double dFactor = 0.04;

void driveToPointSoft(Vector v, int minSpeed, int maxSpeed, ROTATION_TARGET rotationTarget, bool rotateAroundBall) {
  maxSpeed = computeMaxSpeed(v, maxSpeed);
  static Vector v_last;
  v.scale_xy(1.0, 1.55);
  int s = v.getRad() * pFactor;
  s -= (v - v_last).getRad() * dFactor;
  if(s < 0) {
    v.rotate(PI);
  }
  int r = selectRotation(rotationTarget, v);
  double emaRot = 1.0;
  if(botInCorner() || eepromData.gameMode == GAMEMODE_STRIKER_WINGER) emaRot = 0.2;
  static int lastRot = 0;
  r = r * emaRot + lastRot * (1.0 - emaRot);
  lastRot = r;
  v_last = v;
  if (rotateAroundBall) {
    // To ensure a rotation around the ball mostly works,
    // the bot has to move not towards the ball but angled.
    // At the highest rotation speed we need an offset angle,
    // while for the lowest rotation speeds none is needed.
    double max_offset = 0.5*PI;
    double p = (double)ABS(r) / (2.0 * (double)MAX_ROTATION); // Here r can be greater than MAX_ROTATION as the BOUND() is called in drive afterwards
    p = BOUND(p, 0.0, 1.0);
    double offset = p * max_offset;
    double sign = (r > 0 ? -1.0 : 1.0); // Always rotate vector away from rotation
    v.rotate(sign * offset);
  }
  
  // Serial.println("vBallSpeed: " + String(minSpeed));
  // Serial.println("vTargetSpeed: " + String(s));
  drive(v.getAlpha(), BOUND(ABS(s), minSpeed, maxSpeed), r); 
}

void driveToPointHard(Vector v, int speed, ROTATION_TARGET rotationTarget, bool rotateAroundBall) {
  speed = computeMaxSpeed(v, speed);
  int r = selectRotation(rotationTarget, v);
  double emaRot = 1.0;
  if(botInCorner() || eepromData.gameMode == GAMEMODE_STRIKER_WINGER) emaRot = 0.2;
  static int lastRot = 0;
  r = r * emaRot + lastRot * (1.0 - emaRot);
  lastRot = r;

  if (rotateAroundBall) {
    // To ensure a rotation around the ball mostly works,
    // the bot has to move not towards the ball but angled.
    // At the highest rotation speed we need an offset angle,
    // while for the lowest rotation speeds none is needed.
    double max_offset = 0.5*PI;
    double p = (double)ABS(r) / (2.0 * (double)MAX_ROTATION); // Here r can be greater than MAX_ROTATION as the BOUND() is called in drive afterwards
    p = BOUND(p, 0.0, 1.0);
    double offset = p * max_offset;
    double sign = (r > 0 ? -1.0 : 1.0); // Always rotate vector away from rotation
    v.rotate(sign * offset);
  }
  drive(v.getAlpha(), speed, r); 
}

void driveAwayFromLine(ROTATION_TARGET rotationTarget) {
  Vector vTarget;
  if(!midPointValid) {
    vTarget = Vector::rotate(vLine, PI);
  }
  else {
    double weightMidpoint = 0.0;
    vTarget = (vMidPoint * weightMidpoint) + (Vector::rotate(vLine, PI) * (1.0 - weightMidpoint));
  }
  if(rotationTarget == ROTATION_TARGET_OBJECT) {
    rotationTarget = ROTATION_TARGET_MIDAXIS;
  }
  driveToPointHard(vTarget, LINE_SPEED, rotationTarget, false);
}


void driveToPoint(Vector v, int minSpeed, int maxSpeed, bool soft, ROTATION_TARGET rotationTarget, bool rotateAroundBall, bool ignoreLine = false) {
  if(lineSeen) {
    lastDriveSawLine = true;
    if(!ignoreLine) {
      driveAwayFromLine(rotationTarget);
    }
    return;
  }
  if(soft) {
    driveToPointSoft(v, minSpeed, maxSpeed, rotationTarget, rotateAroundBall);
  }
  else {
    driveToPointHard(v, maxSpeed, rotationTarget, rotateAroundBall);
  }
  lastDriveSawLine = false;
}


double rdm = 0;
void driveRandom() {
  Vector vTarget; 
  int spd = 35;
  ROTATION_TARGET r = ROTATION_TARGET_NONE;
  if (lineSeen)
  { 
    vTarget = Vector::rotate(vLine, PI);
    rdm = random(-100, 100) / 100.0;
  }
  else
  {
    vTarget = Vector(1, 0);
    vTarget.rotate(rdm*PI);
  }
  driveToPoint(vTarget, spd, spd, false, r, false);
}


