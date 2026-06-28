#include <elapsedMillis.h>
#include <Wire.h>

#define ABS(X) ((X) < 0 ? -(X) : (X))
#define SDA 21
#define SCL 22
#define DRIVE_ENA   2

#define DRIVE1_PWM 25
#define DRIVE1_DIR 26

#define DRIVE2_PWM 14
#define DRIVE2_DIR 27

#define DRIVE3_PWM 12
#define DRIVE3_DIR 13

#define DRIVE4_PWM 18
#define DRIVE4_DIR 19

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

bool mode = false;
elapsedMillis driveTimer;
elapsedMillis buttonTimer;
elapsedMillis deadTime;