
// ##### TODO: could be different on another field #####

// driveOutOfCorner
constexpr double arcRadiusOfGoal = 100.0;
constexpr double vecRotToExitCorner = 0.38*PI;
// 

// shouldBotKick
constexpr double precisionKickScale = 3.0;
//

// #####################################################


// ################ CORNER ################
Vector driveOutOfCorner() {
    DRIBBLER_ON;
    Vector vTarget;
    
    if(goalSeen) {
        Vector vGoalToBot = vGoal * (-1);
        Vector vGoalToBotUnit = vGoalToBot.unit();
        
        double a = PI - vGoalToBotUnit.getAlpha();
        vGoalToBotUnit.rotate(a);
        int side = (vGoal.getAlpha() < 0 ? 1 : -1);
        vGoalToBotUnit.rotate(vecRotToExitCorner * side);
        vTarget = vGoal + vGoalToBotUnit * arcRadiusOfGoal;
    }
    else vTarget = vOwnGoal;

    return vTarget;
}
// ########################################


// ################ SCORE #################
bool canScoreBasic() {
    bool ballInFront = ABS(vBall.getAlpha()) < 0.2*PI;
    return hasBall() && ballInFront;
}

static elapsedMillis precisionTimer = 0;
bool shouldBotKick(Vector v, bool precise = false) {
    if(!goalSeen) return false;

    if(hasBallTimer < 200) {
        precisionTimer = 0;
        return false;
    }

    double GoalDistNorm = BOUND((MAX_DIST_TO_GOAL - vGoal.getRad()) / (MAX_DIST_TO_GOAL - MIN_DIST_TO_GOAL), 0.25, 1.0);
    double kickThreshold = MAX_KICK_ANGLE * GoalDistNorm; // nah am Tor ist der Winkel größer und weiter weg strenger

    if (precise) {
        kickThreshold /= precisionKickScale;
    }

    if (ABS(v.getAlpha()) > kickThreshold) precisionTimer = 0;
    
    return (precisionTimer > 200);
}

void testAimedKick() {
    DRIBBLER_ON;
    drive(0, 0, selectRotation(ROTATION_TARGET_SCORE));
    if(shouldBotKick(vGoal, true)) {
        DRIBBLER_OFF;
        kick(KICK_POWER);
    }
}

Vector scoreBasic() {
    DRIBBLER_ON;
    if(shouldBotKick(vGoal)) {
        DRIBBLER_OFF;
        kick(KICK_POWER);
    }
    return vGoal;
}

Vector scoreShooter() {
    DRIBBLER_ON;
    if(shouldBotKick(vGoal)) {
        DRIBBLER_OFF;
        driveToPoint(Vector(), 0, 70, false, ROTATION_TARGET_SCORE, false);
        wait(100);
        kick(KICK_POWER);
    }
    return vGoal;
}

Vector scoreWinger() {
    DRIBBLER_ON;
    Vector vTarget;

    if(vGoal.getRad() > 60.0) {
        int radius = 50;
        int side = (vMidPointCorrected.getAlpha() < 0 ? 1 : -1);
        
        if(vMidPoint.getRad() < radius*0.95) {
            double angleMidAxisMidPoint = Vector::angleOffsetBetween(vMidAxis, vMidPoint);
            double alpha = (angleMidAxisMidPoint < 0.25*PI ? 0.5 : 0.75);
            vTarget = Vector::rotate(vMidPoint, alpha*PI*side);
        }
        else if(vMidPoint.getRad() > radius*1.05) {
            double beta = asin(radius/vMidPoint.getRad());
            vTarget = vMidPoint;
            vTarget.rotate(beta*side);
            vTarget *= cos(beta);
        }
        else {
            vTarget = Vector::rotate(vMidPoint, 0.5*PI*side);
        }
    }
    else {
        vTarget = scoreBasic();
    }
    return vTarget;
}

Vector findFreeCorner(int& target) {
    Vector vTarget;
    Vector vRotatedMidAxisToCorner;
    int side = (vMidPointCorrected.getAlpha() < 0.0 ? 1 : -1);
    vRotatedMidAxisToCorner = Vector::rotate(vMidAxis, 0.2*PI*(side)).unit() * 110.0;
    vTarget = vMidPoint + vRotatedMidAxisToCorner;

    vTarget = findGap(vTarget, CORNER_WIDTH);
    if(vTarget.getRad() != 0.0) {
        target = 1;
        return vTarget;
    }

    vRotatedMidAxisToCorner = Vector::rotate(vMidAxis, 0.2*PI*(-side)).unit() * 110.0;
    vTarget = vMidPoint + vRotatedMidAxisToCorner;

    vTarget = findGap(vTarget, CORNER_WIDTH);
    if(vTarget.getRad() != 0.0) 
    {
        target = 2;
        return vTarget;
    }

    return Vector();
}
Vector scoreInConer(int target)
{
    int side = (vMidPointCorrected.getAlpha() < 0.0 ? 1 : -1);
    if (target == 2) side *= -1;
    Vector vRotatedMidAxisToCorner = Vector::rotate(vMidAxis, 0.2*PI*(side)).unit() * 110.0;
    Vector vTarget = vMidPoint + vRotatedMidAxisToCorner;
    return vTarget;
}

Vector findScoreTarget() {
    Vector vTarget;
    static int target = 0;

    if(!changedHasBallStatus()) {
        switch(target) {
            case 0: vTarget = findGap(vGoal, GOAL_WIDTH); break;
            case 1: vTarget = scoreInConer(target); break;
            case 2: vTarget = scoreInConer(target); break;
        }
    }
    else {
        vTarget = findGap(vGoal, GOAL_WIDTH);

        if(vTarget.getRad() != 0.0) {
            target = 0;
            return vTarget;
        }

        vTarget = findFreeCorner(target);
    }
    return vTarget;
}


Vector scoreWallKick() {
    int side = (vMidPointCorrected.getAlpha() < 0 ? 1 : -1);
    Vector v = Vector::rotate(vMidAxis, 0.25*PI*side).unit();
    bool rightKickAngle = ABS(vMidAxis.getAlpha()) < 0.3*PI && ABS(vMidAxis.getAlpha()) > 0.2*PI;
    if(rightKickAngle) {
        kick(KICK_POWER);
    }
    return v;
}

Vector scoreFlank() {
    DRIBBLER_ON;
    Vector vTarget;
    FieldPosition botPos = getPosOnField();


    Vector v = findNearestEnemy();
    bool enemyInFront = (v.getRad() < 15.0 && ABS(v.getAlpha()) < 0.35*PI);
    if(v.getRad() != 0 && enemyInFront && botPos == NEAR_SIDE_LINE) {
        vTarget = scoreWallKick();
        return vTarget;
    }

    vTarget = findGap(vGoal, GOAL_WIDTH);

    if(vTarget.getRad() == 0) {
        vTarget = scoreBasic();
        return vTarget;
    }

    if(shouldBotKick(vTarget) && vTarget.getRad() != 0.0) {
        DRIBBLER_OFF;
        kick(KICK_POWER);
    }
    return Vector();
}


Vector scoreDynamic() {
    DRIBBLER_ON;
    Vector vTarget;
    FieldPosition botPos = getPosOnField();
    static bool isWingerActiv = false;
    if(changedHasBallStatus()) isWingerActiv = false;

    if(isWingerActiv || (botPos == NEAR_SIDE_LINE && ABS(vMidPointCorrected.getAlpha()) < 0.5*PI)) {
        vTarget = scoreWinger();
        isWingerActiv = true;
        return vTarget;
    }

    vTarget = scoreBasic();
    return vTarget;
}

Vector scoreDynamicObjects() {
    DRIBBLER_ON;
    Vector vTarget;
    FieldPosition botPos = getPosOnField();
    static bool isWingerActiv = false;
    if(changedHasBallStatus()) isWingerActiv = false;
    
    if(isWingerActiv || (botPos == NEAR_SIDE_LINE && ABS(vMidPointCorrected.getAlpha()) < 0.5*PI)) {
        vTarget = scoreWinger();
        isWingerActiv = true;
        return vTarget;
    }

    vTarget = scoreFlank();
    return vTarget;
}


// ########################################


constexpr float MAX_DIST_TO_BALL =  25.0f;

Vector driveBehindBall()
{
    bool isBallInFront = ABS(vBall.getAlpha()) < 0.25*PI;
    bool isBallNear = vBall.getRad() < MAX_DIST_TO_BALL; 

    if (isBallInFront && isBallNear) {
        DRIBBLER_ON;
    } else {
        DRIBBLER_OFF;
    }

    Vector vOffset;
    if (midPointValid) { 
        vOffset = vMidAxis;
    } else if (goalSeen) {
        vOffset = vGoal;
    } else if (ownGoalSeen) {
        vOffset = Vector::rotate(vOwnGoal, PI);
    } else {
        vOffset = Vector();
    }
    vOffset = vOffset.unit();
    Vector vTarget;

    float alpha = vBall.getAlpha();
    float alphaAbs = ABS(alpha);

    vOffset.rotate(alpha * 0.5);

    float ballAngleNorm = (float)BOUND(alphaAbs / (0.5*PI), 0.0f, 1.0f);


    // float smoothBallAngleNorm = ballAngleNorm * ballAngleNorm * (3.0 - 2.0 * ballAngleNorm;
    float smoothBallAngleNorm = pow(ballAngleNorm, 0.2f);

    const float minFactor = 0.45f;
    const float maxFactor = 1.0f;

    float factorBallAngle = minFactor + ((maxFactor - minFactor) * smoothBallAngleNorm);

    vOffset *= factorBallAngle * MAX_DIST_TO_BALL;

    if(vOffset.getRad() >= vBall.getRad() && ABS(vBall.getAlpha()) < 0.5*PI) {
        float clampFactor = BOUND(vBall.getRad() / (factorBallAngle * MAX_DIST_TO_BALL), 0.8f, 0.9f);
        vOffset = vOffset.unit() * vBall.getRad() * clampFactor;
    }
    
    vTarget = vBall - vOffset;

    vTarget += Vector(vBallVelocity.getX(), 0.0) * 0.5;

    static Vector vTargetSmooth;
    static bool init = false;
    const float targetEMA = 1.0f;

    if (!init) {
        vTargetSmooth = vTarget;
        init = true;
    } else {
        vTargetSmooth = vTarget * targetEMA + vTargetSmooth * (1.0f - targetEMA);
    }
    return vTargetSmooth;
}

struct StrikerConfig {
    int behindBallMinSpd = vBall.getRad() * 1.35;
    int behindBallMaxSpd = 70;
    bool driveBehindBallSoft = true;
    ROTATION_TARGET driveBehindBallRotation = ROTATION_TARGET_MIDAXIS;
    

    int directBallMinSpd = 5;
    int directBallMaxSpd = 10;
    ROTATION_TARGET directBallRotation = ROTATION_TARGET_MIDAXIS_BALL;

    int scoreMinSpd = 30;
    int scoreMaxSpd = 80;
    ROTATION_TARGET scoreRotation = ROTATION_TARGET_SCORE;

    int cornerMaxSpd = 30;
    ROTATION_TARGET cornerRotation = ROTATION_TARGET_SCORE;

    int minSpdToMidPoint = 0;
    int maxSpdToMidPoint = 50;

    bool lonelyStrikerActive = true;

    bool (*canScore)() = canScoreBasic;
    bool (*botInCornerFn)() = botInCorner;
    Vector (*scoringTarget)() = scoreBasic;
};
