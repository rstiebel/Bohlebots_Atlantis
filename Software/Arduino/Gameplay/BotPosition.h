
FieldPosition getPosOnField() {

    if(!midPointValid) {
        return UNVALID_POS;
    }

    float angleBetweenGoals = (float)Vector::angleOffsetBetween(vGoal, vOwnGoal);
    if(angleBetweenGoals < 1.71f) {
        return IN_CORNER;
    }
    else if(angleBetweenGoals < 2.1f) {
        return NEAR_SIDE_LINE;
    }
    else {
        return MIDDLE;
    }
    return MIDDLE;
}

bool botInCorner() {
    FieldPosition botPos = getPosOnField();
    bool isGoalNear = vGoal.getRad() < 120.0;
    bool isMidPointBehind = ABS(vMidPointCorrected.getAlpha()) > 0.6*PI;
    return ((botPos == IN_CORNER || botPos == NEAR_SIDE_LINE) && (isGoalNear) && (isMidPointBehind) && midPointValid);
}


Vector moveAroundLine(Vector vTargetPoint) {
  Vector vTarget;
  int radius = KEEPER_RADIUS;

  if(midPointValid) {
    double angleMidAxis = Vector::angleOffsetBetween(vMidAxis, vOwnGoal*(-1));
    double angle = BOUND(angleMidAxis / (0.5*PI), 0.0, 1.0);

    angle = 4.0*pow(angle, 2);
    angle = BOUND(angle, 0.0, 1.0);

    const double minFactor = MIN_FACTOR_RADIUS;
    const double maxFactor = 1.0;

    double factor = minFactor + ((maxFactor - minFactor) * angle);
    radius *= factor;
  }

  Vector vOwnGoalToTargetPoint = vTargetPoint - vOwnGoal;
  vOwnGoalToTargetPoint = vOwnGoalToTargetPoint.unit() * radius;
  vTarget = vOwnGoal + vOwnGoalToTargetPoint;

  Vector vOppositeOwnGoal = vOwnGoal * (-1);
  if(midPointValid) {
    int length = vTarget.getRad();
    if(vOppositeOwnGoal.getRad() < radius*0.95) {
      vTarget = vTargetPoint;
    }
    else if(vOppositeOwnGoal.getRad() > radius*1.05) {
      double beta = asin(radius/vOppositeOwnGoal.getRad());
      vTarget = vOppositeOwnGoal;
      int side = vTargetPoint.sideOf(vOppositeOwnGoal);
      vTarget.rotate(beta*side);
      vTarget *= cos(beta) * (-1);
    }
    else {
      int side = vTargetPoint.sideOf(vOwnGoal);
      vTarget = Vector::rotate(vOppositeOwnGoal, 0.5*PI*side);
    }
    vTarget = vTarget.unit() * length * 1.0;
  }

  return vTarget;
}

Vector neutralPositionOffensiveStriker() {
  return vMidPoint;
}

Vector neutralPositionOffensiveKeeper() {
  Vector vTarget;
  Vector vOwnGoalToMidPoint = vMidPoint - vOwnGoal;
  vOwnGoalToMidPoint = vOwnGoalToMidPoint.unit() * KEEPER_RADIUS * MIN_FACTOR_RADIUS;
  vTarget = vOwnGoal + vOwnGoalToMidPoint;
  vTarget = moveAroundLine(vTarget);
  return vTarget;
}

Vector neutralPositionOffensive() {
  if(eepromData.gameMode == GAMEMODE_KEEPER_BASIC || eepromData.gameMode == GAMEMODE_KEEPER_BLOCKING) {
    return neutralPositionOffensiveKeeper();
  }
  else {
    return neutralPositionOffensiveStriker();
  }
}

Vector neutralPositionDefensive() {
  Vector vTarget;
  Vector vOwnGoalToMidPoint = vMidPoint - vOwnGoal;
  vOwnGoalToMidPoint = vOwnGoalToMidPoint.unit() * KEEPER_RADIUS * MIN_FACTOR_RADIUS;
  if(eepromData.gameMode == GAMEMODE_KEEPER_BASIC || eepromData.gameMode == GAMEMODE_KEEPER_BLOCKING) {
    vOwnGoalToMidPoint.rotate(0.18*PI*receivedData.sideOfLastBall);
  }
  else {
    vOwnGoalToMidPoint.rotate(0.18*PI*(-sideOfBall));
    vOwnGoalToMidPoint *= 1.2;
  }
  vTarget = vOwnGoal + vOwnGoalToMidPoint;
  vTarget = moveAroundLine(vTarget);
  return vTarget;
}

void neutralPosition(int minSpd, int maxSpd) {
  DRIBBLER_OFF;
  Vector vTarget;
  ROTATION_TARGET r = ROTATION_TARGET_MIDAXIS;
  switch(eepromData.neutralPosition) {
    case NEUTRAL_POSITION_OFFENSIVE: vTarget = neutralPositionOffensive(); break;
    case NEUTRAL_POSITION_DEFENSIVE: vTarget = neutralPositionDefensive(); break;
    default: break;
  }

  if(aloneOnField) {
    vTarget = neutralPositionOffensiveKeeper();
  }
  int spd = BOUND(vTarget.getRad(), minSpd, maxSpd);
  if(eepromData.gameMode == GAMEMODE_KEEPER_BASIC || eepromData.gameMode == GAMEMODE_KEEPER_BLOCKING) {
    r = ROTATION_TARGET_OWNGOAL;
  }
  drive(vTarget.getAlpha(), spd, selectRotation(r));
}
