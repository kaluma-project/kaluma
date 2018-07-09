#include <SoftwareSerial.h>

int RX = 10;
int TX = 11;

SoftwareSerial ss = SoftwareSerial(RX, TX);

void setup() {
  ss.begin(9600);
  Serial.begin(9600);
}

void loop() {
  if (ss.available()) {
    char inByte = ss.read();
    Serial.write(inByte);
  }
}
