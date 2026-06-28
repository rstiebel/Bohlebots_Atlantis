
double decodeValue(byte p) {
  double normalized = p / 255.0;
  return normalized * 2.0 * maxValueXY - maxValueXY;
}

int bodenUpdate() {
  lineSeen = false;
  lineJumped = false;
  vLine = Vector();

  Wire.requestFrom(BODEN_ADD, numOfReceivedBytes);
  if(Wire.available() < numOfReceivedBytes) return -1;

  byte paket[numOfReceivedBytes];
  for(int i = 0; i < numOfReceivedBytes; i++) {
    paket[i] = Wire.read();
  }

  lineSeen = (paket[0] & 64) != 0;
  lineJumped = (paket[0] & 32) != 0;

  double xLine = decodeValue(paket[1]);
  double yLine = decodeValue(paket[2]);
  vLine = Vector(xLine, yLine);
  return 0;
}