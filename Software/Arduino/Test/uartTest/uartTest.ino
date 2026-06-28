#define RXD2 16
#define TXD2 17
void setup() {
  // Serielle Schnittstelle initialisieren
  Serial.begin(115200);  // Baudrate muss mit Python übereinstimmen
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  Serial.println("Arduino bereit zum Empfangen");
}

void loop() {
  // Prüfen, ob Daten verfügbar sind
  if (Serial2.available() > 0) {
    // Eine Zeile von der seriellen Schnittstelle lesen
    String received = Serial2.readStringUntil('\n'); // '\n' als Trenner
    received.trim(); // Whitespace entfernen
    Serial.print("Empfangen: ");
    Serial.println(received);
  }
}
