#include <elapsedMillis.h>
#include <Wire.h>
#include "Vector.h"

#define MAX(X,Y) ((X) < (Y) ? (Y) : (X))
#define MIN(X,Y) ((X) > (Y) ? (Y) : (X))
#define BOUND(X, LOWER, UPPER) MIN(MAX(LOWER, X), UPPER)
#define ABS(X) ((X) < 0 ? -(X) : (X))

#define I2C_DEV_ADDR 0x30

// TODO
#define threshold 140
#define radius 75.0
#define numOfSensors 32 
#define MAX_CLOUDS 6
#define led_addr 2

// Multiplexer Address
#define add0 13
#define add1 27
#define add2 14

#define SDA 4 // TODO
#define SCL 5 // TODO

int sensor[] = {26, 33, 32, 25}; // TODO
int countJumpedLine = 0;
double const minAngleJumped = 0.61*PI; // 110 degree
double lastRad = 7.0;

boolean lineSeen = false;
boolean lineJumped = false;
boolean debug = true;

byte package[3];

struct Cloud {
  int start;
  int end;
};