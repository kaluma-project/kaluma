#include <SPI.h>

char buf[100];
volatile byte pos;
volatile boolean process_it;

void setup() {
  Serial.begin(9600);
  pinMode(MISO, OUTPUT);
  SPCR |= _BV(SPE);
  SPCR &= ~_BV(MSTR);
  SPCR |= _BV(SPIE);
  pos = 0;
  process_it = false;
  Serial.print("ready...\n");
}

ISR (SPI_STC_vect)
{
  byte c = SPDR;
  if (pos < sizeof(buf)) {
    buf[pos++] = c;
    if (c == '\n') {
      process_it = true;
    }
  }
}

void loop() {
  if (process_it) {
    buf[pos] = 0;
    Serial.print(buf);
    pos = 0;
    process_it = false;
  }
}
