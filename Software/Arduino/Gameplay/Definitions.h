#include <elapsedMillis.h>
#include <Wire.h>

#define ABS(X) ((X) < 0 ? -(X) : (X))
#define MAX(X,Y) ((X) < (Y) ? (Y) : (X))
#define MIN(X,Y) ((X) > (Y) ? (Y) : (X))
#define BOUND(X, LOWER, UPPER) MIN(MAX(LOWER, X), UPPER)
#define SIDE(X) ((X) / ABS(X))

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

#define DRIBBLER_ON  dribbler(100)
#define DRIBBLER_OFF dribbler(0)

#define KICKER_PIN 15
#define KICK_POWER 10

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

enum GAMEMODE : byte {
  GAMEMODE_STRIKER_BASIC = 0,
  GAMEMODE_STRIKER_WINGER,
  GAMEMODE_STRIKER_DYNAMIC,
  GAMEMODE_STRIKER_FLANK,
  GAMEMODE_STRIKER_DYNAMIC_OBJECTS,
  GAMEMODE_KEEPER_BASIC,
  GAMEMODE_KEEPER_BLOCKING
};

enum TESTMODE : byte {
  TESTMODE_NONE = 0,
  TESTMODE_KICK,
  TESTMODE_DRIBBLER,
  TESTMODE_DRIVE_STRAIGHT,
  TESTMODE_DRIVE_RANDOM,
  TESTMODE_MIDAXIS,
  TESTMODE_AIMED_KICK
};

enum NEUTRAL_POSITION : byte {
  NEUTRAL_POSITION_OFFENSIVE = 0,
  NEUTRAL_POSITION_DEFENSIVE
};

int  tastLedID[8]    = {  0x20,  0x21,   0x22,  0x23,  0x24,  0x25,  0x26, 0x27 };
bool portena[8]      = { false, false, false, false, false, false, false, false };
bool taster1Array[8] = { false, false, false, false, false, false, false, false };
bool taster2Array[8] = { false, false, false, false, false, false, false, false };
int  led1Array[8]    = {     0,     0,     0,     0,     0,     0,     0,     0 };
int  led2Array[8]    = {     0,     0,     0,     0,     0,     0,     0,     0 };

int leftModule = 0;
int rightModule = 2;

constexpr int BUTTON_RIGHT = 1;
constexpr int BUTTON_LEFT = 2;


elapsedMillis buttonTimer;
elapsedMillis deadTime;
elapsedMillis kickTimer;
elapsedMillis switchTimer = 2000; 
elapsedMillis hasBallTimer;                                                     

const byte START_BYTE_1 = 255;
const byte START_BYTE_2 = 255;

#define MAX_NUM_BALL 1
#define MAX_NUM_YELLOW 1
#define MAX_NUM_BLUE 1
#define MAX_NUM_OBJECTS 3

#define NUM_OF_PERMS 6


byte ball[MAX_NUM_BALL][2];
byte yellow[MAX_NUM_YELLOW][2];
byte blue[MAX_NUM_BLUE][2];
byte objects[MAX_NUM_OBJECTS][2];

Vector vBall;
Vector vBallVelocity;
Vector vLastBall;

int sideOfBall = 0;
Vector vGoal;
Vector vOwnGoal;

Vector vMidPoint;
Vector vMidPointCorrected;
Vector vMidAxis;

struct Enemy {
  Vector pos;
  Vector velocity;
  Vector relativPosToOwnGoal;
  int velocityStabelCount;
  bool movingTowardsGoal;
  bool onField;
};

Enemy enemies[MAX_NUM_OBJECTS] = {
  {Vector(), Vector(), Vector(), false},
  {Vector(), Vector(), Vector(), false},
  {Vector(), Vector(), Vector(), false}
};

int combinations[NUM_OF_PERMS][MAX_NUM_OBJECTS] = {
  {0,1,2},
  {0,2,1},
  {1,0,2},
  {1,2,0},
  {2,0,1},
  {2,1,0}
};

double cost[MAX_NUM_OBJECTS][MAX_NUM_OBJECTS];

bool yellowSeen = false;
bool blueSeen = false;
bool ballSeen = false;
bool enemySeen = false;

#define midPointValid (yellowSeen && blueSeen)
#define goalSeen      (( eepromData.scoreOnYellow && yellowSeen) || (!eepromData.scoreOnYellow && blueSeen))
#define ownGoalSeen   ((!eepromData.scoreOnYellow && yellowSeen) || ( eepromData.scoreOnYellow && blueSeen))
#define aloneOnField  (!receivedData.play || !communicationValid)

Vector vLine = Vector();
Vector vLastSeenLineBeforeJumped = Vector();
double static maxValueXY = 75.0;
bool lineSeen = false;
bool lineJumped = false;

bool driveBackwards = false;

enum FieldPosition {
  MIDDLE,
  NEAR_SIDE_LINE,
  IN_CORNER,
  UNVALID_POS
};


// TODO: could be different on another field 

#define MAX_ROTATION (20) // Drive.h and Motors.h

// Striker.h and StrikerBackwards.h
const double MAX_KICK_ANGLE = 0.08*PI;
#define MIN_DIST_TO_GOAL 0.0
#define MAX_DIST_TO_GOAL 250.0
// 

#define GOAL_WIDTH 55 // eigentlich 60 aber als puffer etwas weniger
#define OBJECT_WIDTH 22
#define CORNER_WIDTH 20

// Communication.h and Keeper.h
#define MIN_FACTOR_RADIUS1 0.79
#define MIN_FACTOR_RADIUS2 0.75

#define KEEPER_RADIUS1 65 // Schwarzer bot
#define KEEPER_RADIUS2 52 // grauer bot
int KEEPER_RADIUS;
double MIN_FACTOR_RADIUS;

int lastGameMode;
// 


// Drive.h and Keeper.h
#define LINE_SPEED 20
bool lastDriveSawLine = false;

