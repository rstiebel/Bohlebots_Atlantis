
void Test() {
    Vector vTarget;
    int minSpd = 0;
    int maxSpd = 0;
    bool soft = false;
    ROTATION_TARGET r = ROTATION_TARGET_MIDAXIS;

    bool rotateAroundBall = false;
    bool ignoreLine = false;

    FieldPosition botPos = getPosOnField();
    if((botPos == IN_CORNER || (botPos == NEAR_SIDE_LINE && vGoal.getRad() < 113.0)) && midPointValid) {
        vTarget = driveOutOfCorner();
        maxSpd = 15;
        r = ROTATION_TARGET_SCORE;
    }
    // vTarget = scoreWinger();
    driveToPoint(vTarget, minSpd, maxSpd, soft, r, rotateAroundBall, ignoreLine);

}


void Striker(StrikerConfig cfg) {
    Vector vTarget;
    int minSpd = 0;
    int maxSpd = 0;
    bool soft = false;
    ROTATION_TARGET r = ROTATION_TARGET_MIDAXIS;

    FieldPosition botPos = getPosOnField();

    bool rotateAroundBall = false;
    bool botHasBall = hasBall();
    bool ignoreLine = false;


    if(aloneOnField && cfg.lonelyStrikerActive) {
        bool isBallOnSideOfBot = ABS(vBall.getAlpha()) > 0.2 * PI;
        bool inKeeperCircle = (vOwnGoal.getRad() < calculateKeeperRadius() * 1.05);
        bool midPointInFront = (ABS(vMidPoint.getAlpha()) < 0.4*PI);
        if(isBallOnSideOfBot && inKeeperCircle && midPointInFront && !botHasBall) {
            KeeperBasic();
            return;
        }
    }

    // START DRIVE BEHIND BALL
    if(ballSeen && (goalSeen || ownGoalSeen)) {
        vTarget = driveBehindBall();
        minSpd = cfg.behindBallMinSpd; //cfg.behindBallMinSpd;
        maxSpd = cfg.behindBallMaxSpd;
        r = ((botPos == IN_CORNER || botPos == NEAR_SIDE_LINE) ? ROTATION_TARGET_MIDAXIS_BALL : cfg.driveBehindBallRotation);
        soft = cfg.driveBehindBallSoft;
    // END DRIVE BEHIND 

        // START DRIVE DIRECTLY TO BALL
        bool ballRightInFront = ABS(vBall.getAlpha()) < 0.05*PI;
        bool ballNearBot = vBall.getRad() < 15.0;

        if(ballRightInFront && ballNearBot && !botHasBall) {
            vTarget = vBall;
            minSpd = cfg.directBallMinSpd;
            maxSpd = cfg.directBallMaxSpd;
            r = cfg.directBallRotation;
        }
        // END DRIVE DIRECTLY TO BALL

        // START SCORE
        if(cfg.canScore()) {
            vTarget = cfg.scoringTarget();
            minSpd = cfg.scoreMinSpd;
            maxSpd = cfg.scoreMaxSpd;
            r = cfg.scoreRotation;
            rotateAroundBall = true;
            soft = false;
 
            // START DRIVE OUT OF CORNER
            if(cfg.botInCornerFn()) {
                vTarget = driveOutOfCorner();
                maxSpd = cfg.cornerMaxSpd;
                r = cfg.cornerRotation;
                rotateAroundBall = true;
            }
            // END DRIVE OUT OF CORNER
        }
        else hasBallTimer = 0;
        // END SCORE
    }
    else {
        neutralPosition(cfg.minSpdToMidPoint, cfg.maxSpdToMidPoint);
        return;
    }
    driveToPoint(vTarget, minSpd, maxSpd, soft, r, rotateAroundBall, ignoreLine);
} 
