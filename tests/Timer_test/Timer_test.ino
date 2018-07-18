#include <neotimer.h>

Neotimer testTimer = Neotimer(1000);
int Lauf = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(millis());
  testTimer.start();
}

void loop() {
  if(testTimer.done()){
    Lauf++;
    Serial.println(Lauf);
    testTimer.start();
  }
}

