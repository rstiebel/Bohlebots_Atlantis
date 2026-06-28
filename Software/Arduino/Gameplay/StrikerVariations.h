
/*
 * STRIKER VARIATIONS
 * 
 * Create a new variation:
 *   void StrikerXxx() {
 *       StrikerConfig cfg;
 *       cfg.xxx = ...;
 *       Striker(cfg);
 *   }
 *
 * Available parameters:
 * 
 * -- Drive behind ball --
 *   behindBallMinSpd      (default:  0)   Minimum speed
 *   behindBallMaxSpd      (default: 70)   Maximum speed
 *   driveBehindBallSoft   (default: true) Soft braking
 *
 * -- Drive directly to ball --
 *   directBallMinSpd      (default:  5)   Minimum speed
 *   directBallMaxSpd      (default: 10)   Maximum speed
 *   directBallRotation    (default: ROTATION_TARGET_MIDAXIS_BALL)
 *
 * -- Scoring --
 *   scoreMinSpd           (default: 30)   Minimum speed
 *   scoreMaxSpd           (default: 80)   Maximum speed
 *   scoreRotation         (default: ROTATION_TARGET_SCORE)
 *   canScore              (default: canScoreBasic)  Function that decides whether to score
 *   scoringTarget         (default: scoreBasic)     Function that calculates the scoring target
 *
 * -- Corner --f
 *   cornerMaxSpd          (default: 30)   Maximum speed -> after computeSpeed 30 * 0.5 = 15
 *   cornerRotation        (default: ROTATION_TARGET_MIDAXIS)
 *
 * -- MidPoint --
 *   minSpdToMidPoint      (default:  0)   Minimum speed
 *   maxSpdToMidPoint      (default: 50)   Maximum speed
 */


/*
weitere geplante Varianten
-> für Verteitigung von Rückwertsfahren neue Variable einführen (wie beim lonelyStriker)
-> kicken über Bande muss noch verbessert werden 
-> präzise am Gegner vorbei kicken
-> je nachdem wo der bot auf dem Feld steht über außen fahren (Winger)
-> variante die alles beinhaltet
*/

void StrikerBasic() {
    StrikerConfig cfg;
    Striker(cfg);
}

void StrikerShooter() {
    StrikerConfig cfg;
    cfg.scoreMinSpd = 0;
    cfg.scoreMaxSpd = 0;
    cfg.scoringTarget = scoreShooter;
    Striker(cfg);
}

void StrikerSlow() {
    StrikerConfig cfg;
    cfg.behindBallMaxSpd = 50;
    Striker(cfg);
}

void StrikerWinger() {
    StrikerConfig cfg;
    cfg.scoreMinSpd = 5;
    cfg.scoreMaxSpd = 20;
    cfg.scoreRotation = (vGoal.getRad() < 60 ? ROTATION_TARGET_SCORE : ROTATION_TARGET_SIDE);
    cfg.scoringTarget = scoreWinger;
    Striker(cfg);
}

void StrikerFlank() {
    StrikerConfig cfg;
    cfg.scoreMinSpd = 60;
    cfg.scoreMaxSpd = 60;
    cfg.scoringTarget = scoreFlank;
    cfg.scoreRotation = ROTATION_TARGET_OBJECT;
    Striker(cfg);
    lastHasBall = hasBall();
    // TestFlank();
}


void StrikerDynamic() {
    StrikerConfig cfg;
    ROTATION_TARGET r = ROTATION_TARGET_OBJECT;
    FieldPosition botPos = getPosOnField();
    static bool isWingerActiv = false;
    if(changedHasBallStatus()) isWingerActiv = false;

    cfg.scoreMinSpd = 80;
    cfg.scoreMaxSpd = 80;

    if((isWingerActiv || (botPos == NEAR_SIDE_LINE && ABS(vMidPointCorrected.getAlpha()) < 0.5*PI)) && hasBall()) {
        r = (vGoal.getRad() < 60 ? ROTATION_TARGET_SCORE : ROTATION_TARGET_SIDE);
        cfg.scoreMinSpd = 20;
        cfg.scoreMaxSpd = 20;
        isWingerActiv = true;
    }
    cfg.scoringTarget = scoreDynamic;
    cfg.scoreRotation = r;
    Striker(cfg);
    lastHasBall = hasBall();
}

void StrikerDynamicObjects() {
    StrikerConfig cfg;
    ROTATION_TARGET r = ROTATION_TARGET_OBJECT;
    FieldPosition botPos = getPosOnField();
    static bool isWingerActiv = false;
    if(changedHasBallStatus()) isWingerActiv = false;

    cfg.scoreMinSpd = 80;
    cfg.scoreMaxSpd = 80;

    if((isWingerActiv || (botPos == NEAR_SIDE_LINE && ABS(vMidPointCorrected.getAlpha()) < 0.5*PI)) && hasBall()) {
        r = (vGoal.getRad() < 60 ? ROTATION_TARGET_SCORE : ROTATION_TARGET_SIDE);
        cfg.scoreMinSpd = 20;
        cfg.scoreMaxSpd = 20;
        isWingerActiv = true;
    }
    cfg.scoringTarget = scoreDynamicObjects;
    cfg.scoreRotation = r;
    Striker(cfg);
    lastHasBall = hasBall();
}