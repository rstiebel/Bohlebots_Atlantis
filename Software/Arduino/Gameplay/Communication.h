#include <WiFi.h>
#include <esp_now.h>

portMUX_TYPE myMux = portMUX_INITIALIZER_UNLOCKED;

uint8_t addressBot1[] = {0xC8, 0x2E, 0x18, 0xF8, 0xA1, 0xB8};
uint8_t addressBot2[] = {0xC8, 0x2E, 0x18, 0xF8, 0xDB, 0xB8};
//uint8_t addressBot1[] = {0xC8, 0x2E, 0x18, 0xF8, 0x92, 0x84};
//uint8_t addressBot2[] = {0xF8, 0xB3, 0xB7, 0x20, 0x72, 0x10};
uint8_t macAddress[6] = {};

bool doRoleSwitch = true;

typedef struct struct_message {
  Vector vBall;
  Vector vOwnGoal;
  Vector vGoal;

  int sideOfLastBall;
  int GameMode;

  bool roleSwitch;
  bool play;
  bool hasBall;
  bool BallSeen;
  bool OwnGoalSeen;
  bool GoalSeen;

} struct_message;

struct_message myData;
struct_message uncertainReceivedData;
struct_message receivedData;

bool communicationValid = false;

void OnDataSend(const wifi_tx_info_t* mac_addr, esp_now_send_status_t status) {
  (void)mac_addr; // Required by the interrupt handling API, but we ignore it on purpose.
  (void)status;   // Required by the interrupt handling API, but we ignore it on purpose.
  static int counter = 0;
  bool byteReceived = (status == ESP_NOW_SEND_SUCCESS ? true : false);
  if (!byteReceived) counter++;
  else counter = 0;

  communicationValid = ((byteReceived || counter < 3) ? true : false);
  //Serial.println("sendStatus " + String(status == ESP_NOW_SEND_SUCCESS ? true : false));
}

void OnDataRecv(const esp_now_recv_info_t* mac_addr, const uint8_t* incomingData, int len) { // Callback Recv
  (void)mac_addr; // Required by the interrupt handling API, but we ignore it on purpose.
  memcpy(&uncertainReceivedData, incomingData, sizeof(uncertainReceivedData));
  //Serial.println("received");
}

bool checkRoleSwitch() // Striker to Keeper
{
  bool roleSwitch = false;
  if (switchTimer > 2000)
  {
    if (!receivedData.play) roleSwitch = true;
    if (hasBall()) roleSwitch = true;  // abfragen ob auch kamera den ball sieht damit nicht ein anderer Bot die Lichtschranke auslöst
    if(ballSeen && ABS(vBall.getAlpha()) < 0.3*PI && vBall.getRad() < 30.0 && (ABS(receivedData.vBall.getAlpha()) > (ABS(vBall.getAlpha()) + 0.1*PI) || !receivedData.BallSeen) && (vBall.getRad()+10) < receivedData.vBall.getRad()) roleSwitch = true;  //ball distanz anpassen
  }
  if (roleSwitch) {lastGameMode = eepromData.gameMode; eepromData.gameMode = receivedData.GameMode; switchTimer = 0; EEPROM_Write(eepromData);}
  return roleSwitch;
}

void sendComData()
{
  myData.GameMode = eepromData.gameMode;
  myData.play = eepromData.play; 
  myData.hasBall = hasBall(); 
  myData.BallSeen = ballSeen;
  myData.sideOfLastBall = sideOfBall;
  myData.OwnGoalSeen = ownGoalSeen;
  myData.GoalSeen = goalSeen;
  myData.vBall =  correctedVector(vBall);
  myData.vOwnGoal = correctedVector(vOwnGoal);
  myData.vGoal = correctedVector(vGoal);

  if (communicationValid && eepromData.play && doRoleSwitch)
  {
    if (eepromData.gameMode == GAMEMODE_KEEPER_BASIC || eepromData.gameMode == GAMEMODE_KEEPER_BLOCKING) {
      myData.roleSwitch = checkRoleSwitch();
    }
    else myData.roleSwitch = false;
    if (switchTimer < 100) {
      myData.roleSwitch = true;
      myData.GameMode = lastGameMode;
      }
  }
  else if (!communicationValid && eepromData.play && doRoleSwitch && (eepromData.gameMode == GAMEMODE_KEEPER_BASIC || eepromData.gameMode == GAMEMODE_KEEPER_BLOCKING)) {eepromData.gameMode = receivedData.GameMode; switchTimer = 0; EEPROM_Write(eepromData);}
   
  

  
  /*esp_err_t result =*/ esp_now_send(macAddress, (uint8_t*)&myData, sizeof(myData));

  /*if (result == ESP_OK) {
    Serial.println("Sending confirmed");
  }
  else {
    Serial.println("Sending error");
  }*/

}

void readComData()
{
  portENTER_CRITICAL(&myMux);
  memcpy(&receivedData, &uncertainReceivedData, sizeof(uncertainReceivedData));
  portEXIT_CRITICAL(&myMux);
  if ((eepromData.gameMode != GAMEMODE_KEEPER_BASIC && eepromData.gameMode != GAMEMODE_KEEPER_BLOCKING) && eepromData.play && receivedData.roleSwitch) {eepromData.gameMode = receivedData.GameMode; switchTimer = 200; EEPROM_Write(eepromData);}
  /*receivedData.vBall = correctedVector(receivedData.vBall);
  receivedData.vGoal = correctedVector(receivedData.vGoal);
  receivedData.vOwnGoal = correctedVector(receivedData.vOwnGoal);
  */
   /*if (communicationValid && receivedData.roleSwitch)
  {
    
    Serial.println("Data received: ");
    Serial.print("switchTimer: ");
    Serial.println(switchTimer);
    Serial.print("roleSwitch: ");
    Serial.println(receivedData.roleSwitch);
   
    Serial.print("play: ");
    Serial.println(receivedData.play);
    Serial.print("hasBall: ");
    Serial.println(receivedData.hasBall);
    Serial.print("ballSeen: ");
    Serial.println(receivedData.BallSeen);
    Serial.print("vBall: ");
    Serial.println(receivedData.vBall.debug_string());
    Serial.print("OwnGoalSeen: ");
    Serial.println(receivedData.OwnGoalSeen);
    Serial.print("vOwnGoal: ");
    Serial.println(receivedData.vOwnGoal.debug_string());
    Serial.print("GoalSeen: ");
    Serial.println(receivedData.GoalSeen);
    Serial.print("vGoal: ");
    Serial.println(receivedData.vGoal.debug_string());
    Serial.println();
  }*/
}

void setMacAddress()
{
  WiFi.begin();
  uint8_t myMac[6]; 
  WiFi.macAddress(myMac);

  memcpy(macAddress, addressBot2, 6);
  KEEPER_RADIUS = KEEPER_RADIUS1;
  MIN_FACTOR_RADIUS = MIN_FACTOR_RADIUS1;
  for (int i = 0; i < 6; i++) {
    if (myMac[i] != addressBot1[i]) {
      memcpy(macAddress, addressBot1, 6);
      KEEPER_RADIUS = KEEPER_RADIUS2;
      MIN_FACTOR_RADIUS = MIN_FACTOR_RADIUS2;
      break;
    }
  }
  WiFi.disconnect();
}

void initCom()
{
  WiFi.mode(WIFI_STA);
  setMacAddress();
  WiFi.setTxPower(WIFI_POWER_15dBm); // 15dpm -17dpm um unter 100 mW EIRP zu landen
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSend); //print Callback-Send 
  esp_now_register_recv_cb(OnDataRecv);//print Callback-Recv

 
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, macAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
  Serial.println("Peer Add Failed");
  return;
  }
}

void comUpdate()
{
  sendComData();
  readComData();
}

/* //Macadresse herraus finden 
#include <WiFi.h>

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin();              // wichtig!
  delay(100);

  Serial.println(WiFi.macAddress());
}

void loop() {}
*/