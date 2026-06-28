#define RXD2 16
#define TXD2 17

const byte START_BYTE_1 = 255;
const byte START_BYTE_2 = 255;

#define MAX_NUM_BALL 1
#define MAX_NUM_YELLOW 1
#define MAX_NUM_BLUE 1
#define MAX_NUM_LINES 8
#define MAX_NUM_OBJECTS 4

byte ball[MAX_NUM_BALL][2];
byte yellow[MAX_NUM_YELLOW][2];
byte blue[MAX_NUM_BLUE][2];
byte lines[MAX_NUM_LINES];
byte objects[MAX_NUM_OBJECTS][2];

Vector vBall;
Vector vBlue;
Vector vYellow;