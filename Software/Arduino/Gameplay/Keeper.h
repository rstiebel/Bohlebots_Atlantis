
int calculateKeeperRadius() {
  int radius = KEEPER_RADIUS;
  if(!midPointValid) return radius;
  double angleMidAxis = Vector::angleOffsetBetween(vMidAxis, vOwnGoal*(-1));
  double angle = BOUND(angleMidAxis / (0.5*PI), 0.0, 1.0);

  angle = 4.0*pow(angle, 2);
  angle = BOUND(angle, 0.0, 1.0);

  const double minFactor = MIN_FACTOR_RADIUS;
  const double maxFactor = 1.0;

  double factor = minFactor + ((maxFactor - minFactor) * angle);
  radius *= factor;
  return radius;
}

Vector getPositionOnArc(Vector vTargetPoint, bool ignoreVelocity = false) {
  Vector vTarget;
  int radius = KEEPER_RADIUS;

  radius = calculateKeeperRadius();

  Vector vOwnGoalToTargetPoint = vTargetPoint - vOwnGoal;
  vOwnGoalToTargetPoint = vOwnGoalToTargetPoint.unit() * radius;
  vTarget = vOwnGoal + vOwnGoalToTargetPoint;

  Vector vOppositeOwnGoal = vOwnGoal * (-1);
  if(midPointValid) {
    int length = vTarget.getRad();
    if(vOppositeOwnGoal.getRad() < radius*0.95) {
      if(ABS(vTargetPoint.getAlpha()) > 0.5*PI) {
        int side = vTargetPoint.sideOf(vOwnGoal);
        vTarget = Vector::rotate(vOppositeOwnGoal, 0.5*PI*side);
      }
      else {
        vTarget = vTargetPoint;
      }
    }
    else if(vOppositeOwnGoal.getRad() > radius*1.05) {
      double beta = asin(radius/vOppositeOwnGoal.getRad());
      vTarget = vOppositeOwnGoal;
      int side = vTargetPoint.sideOf(vOppositeOwnGoal);
      vTarget.rotate(beta*side);
      vTarget *= cos(beta) * (-1);
      if(ABS(vTargetPoint.getAlpha()) > 0.5*PI) {
        vTarget = driveBehindBall();
      }
    }
    else {
      int side = vTargetPoint.sideOf(vOwnGoal);
      vTarget = Vector::rotate(vOppositeOwnGoal, 0.5*PI*side);
    }
    vTarget = vTarget.unit() * length * 1.0;
  }

  if(!ignoreVelocity) vTarget += vTarget.unit() * vBallVelocity.getRad() * 0.5;
  return vTarget;
}

Vector calculateBallPos() {
  Vector vTarget;
  if(receivedData.OwnGoalSeen && ownGoalSeen) {
    Vector vOwnGoalToBall = receivedData.vBall - receivedData.vOwnGoal;
    vTarget = vOwnGoal + vOwnGoalToBall;
  }
  else if(receivedData.GoalSeen && goalSeen) {
    Vector vGoalToBall = receivedData.vBall - receivedData.vGoal;
    vTarget = vGoal + vGoalToBall;
  }
  else {
    vTarget = Vector();
  }
  vTarget = Vector::rotate(vTarget, vMidAxis.getAlpha());
  return vTarget;
}


void KeeperBasic() {
  Vector vTarget;
  Vector vTargetBall;
  bool soft = true;
  bool rotateAroundBall = false;
  bool calculateBallPossible = (doRoleSwitch && !aloneOnField && receivedData.BallSeen && ((goalSeen && receivedData.GoalSeen) || (ownGoalSeen && receivedData.OwnGoalSeen)));

  if(!ballSeen && !calculateBallPossible) {
    neutralPosition(0, 50);
    return;
  }
  if(ballSeen) {
    vTargetBall = vBall;
  }
  else {
    vTargetBall = calculateBallPos();
  }

  if(ABS(vTargetBall.getAlpha()) < 0.3*PI && vTargetBall.getRad() < 30.0) {
    DRIBBLER_ON;
  }
  else {
    DRIBBLER_OFF;
  }

  vTarget = getPositionOnArc(vTargetBall);

  driveToPoint(vTarget, 0, 60, soft, ROTATION_TARGET_OWNGOAL, rotateAroundBall);
}

void KeeperBlocking() {
  Vector vTarget;
  bool soft = true;
  bool rotateAroundBall = false;
  bool calculateBallPossible = (doRoleSwitch && !aloneOnField && receivedData.BallSeen && ((goalSeen && receivedData.GoalSeen) || (ownGoalSeen && receivedData.OwnGoalSeen)));

  if(ballSeen || calculateBallPossible) {
    KeeperBasic();
    return;
  }

  Vector v = findNearestEnemyToOwnGoal(); //findMovingEnemy();
  if(v.getRad() == 0) {
    neutralPosition(0, 50);
    return;
  }
  vTarget = moveAroundLine(v);
  driveToPoint(vTarget, 0, 60, soft, ROTATION_TARGET_OWNGOAL, rotateAroundBall);
}