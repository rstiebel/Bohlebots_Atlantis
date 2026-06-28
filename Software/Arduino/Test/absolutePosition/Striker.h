Vector score() {
  dribbler(100);
  if(ABS(vGoal.getAlpha()) < 0.15 * PI) {
    //kick(KICK_POWER);
  }
  return vGoal;
}

Vector driveBehindBall() {
  if(ABS(vBall.getAlpha()) < 0.25 * PI && vBall.getRad() <= 55) {
    dribbler(100);
  }
  else {
    dribbler(0);
  }
    
  Vector vOffset;
  if(midPointValid) {
    vOffset = vMidAxis;
  }
  else if (goalSeen){
    vOffset = vGoal; //Vector::rotate(vGoal, PI);
  }
  else if (ownGoalSeen){
    vOffset = Vector::rotate(vOwnGoal, PI);
  }
  else vOffset = Vector();

  vOffset = vOffset.unit();
  Vector vTarget;
  double a = vBall.getAlpha() * 0.5;
  vOffset.rotate(a);
  /*
  Mittelwert von den letzten 5 dist nehmen, um Sprung an der Seite zu verhindern
  die Bedingung ((ABS(vBall.getAlpha()) > 0.5) ? 1.0 : 0.85); ändern, um nahe am ball einen kürzeren Vektor zu haben
  */
  double factorBallAngle = ((ABS(vBall.getAlpha()) > 0.5) ? 1.0 : 0.85);
  vOffset *= (factorBallAngle * 40);
  return vTarget = vBall - vOffset;
}

/*
vOffset = vOffset.unit();
Vector vTarget;

double a = vBall.getAlpha() * 0.5;
vOffset.rotate(a);

double ballAngleNorm = BOUND(ABS(vBall.getAlpha()) / (0.5 * PI), 0.0, 1.0);


double smoothBallAngleNorm = ballAngleNorm * ballAngleNorm * (3.0 - 2.0 * ballAngleNorm);


double minFactor = 0.7;
double maxFactor = 1.0;
double factorBallAngle = minFactor + ((maxFactor-minFactor) * smoothBallAngleNorm);


// Offset anwenden
vOffset *= factorBallAngle * 40;

// Zielpunkt berechnen
vTarget = vBall - vOffset;
return vTarget;
*/

void Striker() {
  Vector vTarget;
  int minSpd = 0;
  int maxSpd = 0;
  bool soft = false;
  ROTATION_TARGET r = ROTATION_TARGET_MIDAXIS;

  if(lineSeen) {
    vTarget = driveAwayFromLine();
    maxSpd = 35;
  }
  else if(ballSeen) {
    vTarget = driveBehindBall();
    minSpd = 5;
    maxSpd = 30;
    soft = true;
    if(ABS(vBall.getAlpha()) < 0.05 * PI && vBall.getRad() <= 32) {
      vTarget = score();
      maxSpd = 40;
      soft = false;
      r = ROTATION_TARGET_SCORE;
    }
  }
  else {
    vTarget = vMidPoint;
    minSpd = 3;
    maxSpd = 50;
    soft = true;
  }
  driveToPoint(vTarget, minSpd, maxSpd, soft, r);
} 