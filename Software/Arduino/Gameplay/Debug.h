
void debugPrint() {
  Serial.println("################################ Start ################################");
  Serial.println("################# Ball ################");
  vBall.debug_println();
  Serial.println("");

  Serial.println("################# Goal ################");
  vGoal.debug_println();
  Serial.println("");

  Serial.println("################ ownGoal ##############");
  vOwnGoal.debug_println();
  Serial.println("");

  Serial.println("############# MidpointValid ###########");
  Serial.println(String(midPointValid));

  Serial.println("############### Midpoint ##############");
  vMidPoint.debug_println();
  Serial.println("");

  Serial.println("################ MidAxis ##############");
  vMidAxis.debug_println();
  Serial.println("");

  Serial.println("############### PosOnField ############");
  Serial.println(String(getPosOnField()));
  Serial.println("AngleBetweenGoals: " + String((float)Vector::angleOffsetBetween(vGoal, vOwnGoal)));

  Serial.println("################ hasBall ##############");
  Serial.println(hasBall());
  Serial.println(analogRead(INPUT4));
  Serial.println("");

  Serial.println("################################ Objects ##############################");
  for(int i = 0; i < MAX_NUM_OBJECTS; i++) {
    enemies[i].pos.debug_println();
    enemies[i].velocity.debug_println();
    enemies[i].relativPosToOwnGoal.debug_println();
    Serial.println("stabelCount: " + String(enemies[i].velocityStabelCount));
    Serial.println("isEnemyMovingTowardsGoal: " + String(enemies[i].movingTowardsGoal));
    Serial.println("onField: " + String(enemies[i].onField));
    Serial.println("");
  }
  Serial.println("");


  Serial.println("################# Line ################");
  Serial.println("lineSeen: " + String(lineSeen));
  Serial.println("lineJumped: " + String(lineJumped));
  vLine.debug_println();
  Serial.println("################# End #################");
  Serial.println("");
}