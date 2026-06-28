#include <Wire.h>
#define BODEN_ADD 0x30
#define SDA 21
#define SCL 22

double static radius = 75.0;
bool lineSeen = false;
bool lineJumped = false;
int lineRadius = 0;
