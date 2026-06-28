int rotToMidAxis()
{
  int r = 0;
  if(midPointValid) {
    r = BOUND(vMidAxis.getAlpha() * 20, -20, 20); // turn towards MidAxis
  } else if(goalSeen) {
    r = BOUND(vGoal.getAlpha() * 20, -20, 20); // turn towards Goal
  } else if(ownGoalSeen) {
    r = BOUND(Vector::rotate(vOwnGoal, PI).getAlpha() * 20, -20, 20); // turn away from OwnGoal
  }
  return r;
}

int rotToScore()
{
  int r = 0;
  if(goalSeen) {
    r = BOUND(vGoal.getAlpha() * 12, -20, 20); // turn towards Goal
  } else if(ownGoalSeen) {
    r = BOUND(Vector::rotate(vOwnGoal, PI).getAlpha() * 10, -20, 20); // turn away from OwnGoal
  }
  return r;
}

int rotToBall() {
  if(!ballSeen) {
    return rotToMidAxis();
  }
  double a = vMidAxis.angleOffsetTo(vBall);
  if (vBall.getAlpha() < 0) a = -a;
  a = BOUND(a, -PI/8.0, PI/8.0);
  Vector v = Vector::rotate(vMidAxis, a);
  return BOUND(v.getAlpha() * 10, -20.0, 20.0);
}

enum ROTATION_TARGET {
  ROTATION_TARGET_MIDAXIS,
  ROTATION_TARGET_BALL,
  ROTATION_TARGET_SCORE,
};


int selectRotation(ROTATION_TARGET rotationTarget) {
  int r = 0;
  switch(rotationTarget) {
    case ROTATION_TARGET_MIDAXIS: r = rotToMidAxis(); break;
    case ROTATION_TARGET_BALL: r = rotToBall(); break;
    case ROTATION_TARGET_SCORE: r = rotToScore(); break;
    default: break;
  }
  return r;
}

void driveToPointSoft(Vector v, int minSpeed, int maxSpeed, ROTATION_TARGET rotationTarget) {
  int s = BOUND(v.getRad(), minSpeed, maxSpeed);
  int r = selectRotation(rotationTarget);
  drive(v.getAlpha(), s, r); 
}

void driveToPointHard(Vector v, int speed, ROTATION_TARGET rotationTarget) {
  int r = selectRotation(rotationTarget);
  drive(v.getAlpha(), speed, r); 
}

void driveToPoint(Vector v, int minSpeed, int maxSpeed, bool soft, ROTATION_TARGET rotationTarget) {
  if(soft) {
    driveToPointSoft(v, minSpeed, maxSpeed, rotationTarget);
    return;
  }
  driveToPointHard(v, maxSpeed, rotationTarget);
}

void driveToMidPoint() {
  dribbler(0);
  driveToPointSoft(vMidPoint, 2, 50, ROTATION_TARGET_MIDAXIS);
}


Vector driveAwayFromLine() {
  return Vector::rotate(vLine, PI);
}

