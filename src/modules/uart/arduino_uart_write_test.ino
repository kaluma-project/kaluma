#include <SoftwareSerial.h>

int RX = 10;
int TX = 11;

SoftwareSerial ss = SoftwareSerial(RX, TX);

void setup() {
  ss.begin(9600);
}

void loop() {
  ss.println("Hello, world!!!");
  delay(1000);
}
