#include "definitions.h"
#include "Vector.h"
Vector vLine = Vector();

double decodeValue(byte p) {
  double normalized = p / 255.0;
  return normalized * 2.0 * radius - radius;
}

int bodenUpdate() {
  lineSeen = false;
  lineJumped = false;
  vLine = Vector();

  Wire.requestFrom(BODEN_ADD, 3);
  if(Wire.available() < 3) return -1;

  byte paket[3];
  for(int i = 0; i < 3; i++) {
    paket[i] = Wire.read();
  }

  lineSeen = (paket[0] & 64) != 0;
  lineJumped = (paket[0] & 32) != 0;

  double xLine = decodeValue(paket[1]);
  double yLine = decodeValue(paket[2]);
  vLine = Vector(xLine, yLine);
  return 0;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA, SCL);
}

void loop() {
  bodenUpdate();
  Serial.println("lineSeen: " + String(lineSeen));
  Serial.println("lineJumped: " + String(lineJumped));
  Serial.println(vLine.debug_string());
}