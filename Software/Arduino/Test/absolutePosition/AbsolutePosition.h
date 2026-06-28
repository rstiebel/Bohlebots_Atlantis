
double normalizeAngle(double a) {
  while (a > PI) a -= 2 * PI;
  while (a < -PI) a += 2 * PI;
  return a;
}


Vector computeAbsolutePos(Vector relBotPosToGoal, Vector GoalField) {
  if(!midPointValid) return Vector();
  double diffMidAxis = vMidAxisAbsolute.getAlpha() - vMidAxis.getAlpha(); 
  diffMidAxis = normalizeAngle(diffMidAxis);
  Vector botPosInField = Vector::rotate(relBotPosToGoal, diffMidAxis); // relativer Vektor zum Tor wird nun so rotiert, sodass der Vektor vom roboter aus zum absoluten Tor zeigt 
  Vector absoluteBotPos = GoalField - botPosInField; // man braucht festen Bezugspunkt (GoalField) damit man die genaue Position vom roboter erhält
  return absoluteBotPos;
}