
void initI2C()
{
  Wire.begin(SDA, SCL);
  for (int i = 0; i < 8; i++)
  {
    Wire.beginTransmission(tastLedID[i]);
    byte error = Wire.endTransmission();
    if (error == 0) portena[i] = true;
  }
}

bool button(int device, int nr)                  
{
  if (device < 0) return false;             
  if (device > 7) return false;
  if (nr == 1) return taster1Array[device];
  if (nr == 2) return taster2Array[device];
  return false;
}

void led(int device, int nr, int c) {
  if (c < 0) return;
  if (c > 7) return;
  if (c < 0) return;
  if (c > 7) return;

  if (nr == 1) {
    c = c * 2;
    led1Array[device] = c;
  }
  if (nr == 2) {
    c = c * 16;
    if (c > 63) c = c + 64;
    led2Array[device] = c;
  }
}

void syncTastLed() {
  for (int i = 0; i < 8; i++) {
    if (portena[i]) {
      int ledwert = 255 - led1Array[i] - led2Array[i];
      Wire.beginTransmission(tastLedID[i]);
      Wire.write(ledwert);;
      Wire.endTransmission();
      Wire.requestFrom(tastLedID[i], 1);
      if (Wire.available()) {
        int tread = 255 - Wire.read();
        tread = tread % 128;
        if (tread > 63) taster2Array[i] = true; else taster2Array[i] = false;
        tread = tread % 2;
        if (tread > 0) taster1Array[i] = true; else taster1Array[i] = false;
      }
    }
  }
}

void wait(int timeMillis) {
  deadTime = 0;
  syncTastLed();
  while (deadTime < timeMillis)
  {
    if ((deadTime % 10) == 0)
    {
      syncTastLed();
      readData();
      bodenUpdate();
    }

    else delay(1);
  }
}


void statusBlink(int c)
{
  led(rightModule, 1, c);
  led(rightModule, 2, c);
  led(leftModule, 1, c);
  led(leftModule, 2, c);
}

