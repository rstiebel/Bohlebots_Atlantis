#define ABS(X) ((X) < 0 ? -(X) : (X))
#define DRIVE_ENA   2

#define DRIB_PWM 5
#define DRIB_DIR 4

/*
#define DRIB_PWM 12
#define DRIB_DIR 13
*/
#define SDA 21
#define SCL 22
#include <elapsedMillis.h>
#include <Wire.h>
bool mode = false;

enum COLOR : int {
  OFF = 0,
  GREEN = 1,
  RED = 2,
  YELLOW = 3,
  BLUE = 4,
  CYAN = 5,
  MAGENTA = 6,
  WHITE = 7
};

elapsedMillis buttonTimer;
elapsedMillis deadTime;