#include <elapsedMillis.h>
#include <Wire.h>
#include "Vector.h"

#define MAX(X,Y) ((X) < (Y) ? (Y) : (X))
#define MIN(X,Y) ((X) > (Y) ? (Y) : (X))
#define BOUND(X, LOWER, UPPER) MIN(MAX(LOWER, X), UPPER)
#define ABS(X) ((X) < 0 ? -(X) : (X))

#define I2C_DEV_ADDR 0x30

// TODO
#define threshold 3900
#define numOfSensors 48 
#define MAX_CLOUDS 6
#define MAX_SENSOR_RADIUS 77
#define MIN_SENSOR_RADIUS 48
#define led_addr 2

// Multiplexer Address
#define add0 14
#define add1 27
#define add2 13

#define SDA 21 
#define SCL 22 

int arrSensor[] = {26, 25, 33, 32, 35, 34}; 
int countJumpedLine = 0;
double const minAngleJumped = 0.5*PI; // 110 degree /* 0.61*PI*/
//double lastRad = 7.0;
Vector vFirstLine = Vector();

boolean lineSeen = false;
boolean lineJumped = false;
boolean debug = true;

byte package[3];

struct Sensor {
  bool line;
  const double alpha;
  const int radius;

  Sensor(bool l, double a, int r) 
    : line(l), alpha(a), radius(r) {}
};


Sensor sensors[numOfSensors] = {
  {false,  0.0, 48}, // multiplexer 0 
  {false, 0.26, 48}, // multiplexer 0 
  {false, 0.52, 48}, // multiplexer 0 
  {false, 0.59, 58}, // multiplexer 3
  {false, 0.79, 48}, // multiplexer 0
  {false, 1.05, 48}, // multiplexer 0
  {false, 1.31, 48}, // multiplexer 0
  {false, 1.53, 77}, // multiplexer 3
  {false, 1.57, 48}, // multiplexer 0
  {false, 1.57, 58}, // multiplexer 3
  {false, 1.57, 68}, // multiplexer 3
  {false, 1.61, 77}, // multiplexer 3
  {false, 1.70, 77}, // multiplexer 3
  {false, 1.79, 77}, // multiplexer 3
  {false, 1.83, 48}, // multiplexer 0
  {false, 1.88, 77}, // multiplexer 3
  {false, 2.09, 48}, // multiplexer 1
  {false, 2.36, 48}, // multiplexer 1
  {false, 2.62, 48}, // multiplexer 1
  {false, 2.85, 77}, // multiplexer 4
  {false, 2.88, 48}, // multiplexer 1
  {false, 2.97, 77}, // multiplexer 4
  {false, 3.08, 77}, // multiplexer 4
  {false, 3.14, 48}, // multiplexer 1
  {false, 3.14, 58}, // multiplexer 4
  {false, 3.14, 68}, // multiplexer 4
  {false, 3.20, 77}, // multiplexer 4
  {false, 3.32, 77}, // multiplexer 4
  {false, 3.40, 48}, // multiplexer 1
  {false, 3.43, 77}, // multiplexer 4
  {false, 3.67, 48}, // multiplexer 1
  {false, 3.93, 48}, // multiplexer 1
  {false, 4.19, 48}, // multiplexer 2
  {false, 4.41, 77}, // multiplexer 5
  {false, 4.45, 48}, // multiplexer 2
  {false, 4.49, 77}, // multiplexer 5
  {false, 4.58, 77}, // multiplexer 5
  {false, 4.67, 77}, // multiplexer 5
  {false, 4.71, 48}, // multiplexer 2
  {false, 4.71, 58}, // multiplexer 5
  {false, 4.71, 68}, // multiplexer 5
  {false, 4.76, 77}, // multiplexer 5 
  {false, 4.97, 48}, // multiplexer 2
  {false, 5.24, 48}, // multiplexer 2
  {false, 5.50, 48}, // multiplexer 2
  {false, 5.69, 58}, // multiplexer 5
  {false, 5.76, 48}, // multiplexer 2
  {false, 6.02, 48}  // multiplexer 2
};

struct Cloud {
  int start;
  int end;
};