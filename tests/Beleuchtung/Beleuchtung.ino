#include <neotimer.h>

#define LichtPin 6

unsigned long Dauer = 12L * 60 * 60 * 1000;

Neotimer LichtTimer = Neotimer(Dauer);
Neotimer DunkelTimer = Neotimer(Dauer);
void setup() {
  pinMode(LichtPin, OUTPUT);
  LichtAn();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (LichtTimer.done()) {
    LichtAus();
  }
  if (DunkelTimer.done()) {
    LichtAn();
  }
  //Alle Minute prüfen
  delay(500);
}

void LichtAn() {
  digitalWrite(LichtPin, LOW);
  DunkelTimer.reset();
  LichtTimer.start();
}

void LichtAus() {
  digitalWrite(LichtPin, HIGH);
  LichtTimer.reset();
  DunkelTimer.start();
}

