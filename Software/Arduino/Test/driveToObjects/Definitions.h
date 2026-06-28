#include <elapsedMillis.h>
#include <Wire.h>

#define ABS(X) ((X) < 0 ? -(X) : (X))
#define MAX(X,Y) ((X) < (Y) ? (Y) : (X))
#define MIN(X,Y) ((X) > (Y) ? (Y) : (X))
#define BOUND(X, LOWER, UPPER) MIN(MAX(LOWER, X), UPPER)
#define SDA 21
#define SCL 22
#define RXD2 16
#define TXD2 17

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

int mode = 0;
bool play = false;
elapsedMillis driveTimer;
elapsedMillis buttonTimer;
elapsedMillis deadTime;

const byte START_BYTE_1 = 255;
const byte START_BYTE_2 = 255;

#define MAX_NUM_BALL 1
#define MAX_NUM_YELLOW 1
#define MAX_NUM_BLUE 1
#define MAX_NUM_LINES 8
#define MAX_NUM_OBJECTS 20

byte ball[MAX_NUM_BALL][2];
byte yellow[MAX_NUM_YELLOW][2];
byte blue[MAX_NUM_BLUE][2];
byte lines[MAX_NUM_LINES];
byte objects[MAX_NUM_OBJECTS][2];

Vector vBall;
Vector vGoal;
Vector vOwnGoal;

Vector vMidPoint;
Vector vMidAxis;

bool scoreOnYellow = true;

bool yellowSeen = false;
bool blueSeen = false;
bool ballSeen = false;
bool enemySeen = false;

#define midPointValid (yellowSeen && blueSeen)
#define goalSeen      (( scoreOnYellow && yellowSeen) || (!scoreOnYellow && blueSeen))
#define ownGoalSeen   ((!scoreOnYellow && yellowSeen) || ( scoreOnYellow && blueSeen))
