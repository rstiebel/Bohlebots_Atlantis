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

#define DRIB_PWM 5
#define DRIB_DIR 4

//#define KICKER_PIN 15
//#define KICK_POWER 10

#define INPUT1 35
#define INPUT2 34
#define INPUT3 39
#define INPUT4 36

#define BODEN_ADD 0x30
#define numOfReceivedBytes 3

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
elapsedMillis kickTimer;                                                      

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
Vector vMidAxisAbsolute;

bool scoreOnYellow = true;
bool play = false;
int mode = 0;

bool yellowSeen = false;
bool blueSeen = false;
bool ballSeen = false;
bool enemySeen = false;

#define midPointValid (yellowSeen && blueSeen)
#define goalSeen      (( scoreOnYellow && yellowSeen) || (!scoreOnYellow && blueSeen))
#define ownGoalSeen   ((!scoreOnYellow && yellowSeen) || ( scoreOnYellow && blueSeen))


/*
The playing field is 158 cm by 219 cm. The field is marked by a white line which is part of the playing field.
Around the playing field, beyond the white line, there is an outer area of 12 cm in width.
The floor near the exterior wall includes a wedge, which is an incline with a 10 cm base and 2 +/- 1 cm rise
for allowing the ball to roll back into play when it leaves the playing field. Note that the goal should not
contain the wedge.
Total dimensions of the field, including the outer area, are 182 cm by 243 cm.
*/
const int FIELD_SIZE[2] = {182, 243};
#define distWallGoal 5
Vector vOwnGoalAbsolutePos = Vector(FIELD_SIZE[0] / 2.0, distWallGoal);
Vector vGoalAbsolutePos = Vector(FIELD_SIZE[0] / 2.0, FIELD_SIZE[1] - distWallGoal);

Vector vLine = Vector();
double static maxValueXY = 75.0;
bool lineSeen = false;
bool lineJumped = false;


int leftModule = 0;
int rightModule = 1;

int  tastLedID[8]    = {  0x20,  0x21,   0x22,  0x23,  0x24,  0x25,  0x26, 0x27 };
bool portena[8]      = { false, false, false, false, false, false, false, false };
bool taster1Array[8] = { false, false, false, false, false, false, false, false };
bool taster2Array[8] = { false, false, false, false, false, false, false, false };
int  led1Array[8]    = {     0,     0,     0,     0,     0,     0,     0,     0 };
int  led2Array[8]    = {     0,     0,     0,     0,     0,     0,     0,     0 };
