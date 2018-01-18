#include "pitches.h"
int buzzPin = 11; //                                     ||                                 |                                   ||
int melody[] = {NOTE_C6, 0, NOTE_AS5, 0, NOTE_GS5, 0, NOTE_G5, 0, NOTE_G5, NOTE_GS5, NOTE_GS5, NOTE_GS5, 0, NOTE_G5, NOTE_GS5, NOTE_GS5, NOTE_GS5, 0, NOTE_C6, 0, NOTE_AS5, 0, NOTE_GS5, 0, NOTE_G5, 0, NOTE_GS5, NOTE_GS5, NOTE_GS5, NOTE_C6, NOTE_AS5, NOTE_C6, NOTE_AS5, NOTE_GS5};
long dur[] = {8, 8, 8, 8, 8, 8, 8, 8, 16, 16, 16, 16 / 3, 8, 16, 16, 16, 16 / 3, 8, 8, 8, 8, 8, 8, 8, 8, 8, 16, 16, 16, 16 / 13, 16 / 14, 16, 16, 1};
int speed = 2400;
int M_Leng = sizeof(melody) / 2;
int pause_fest = 25;
float pause_flex = 1;
void setup() {
  //Buzzer
  pinMode(buzzPin, OUTPUT);

  int Length;
  for (int i = 0; i < M_Leng; i++) {
    Length = speed / dur[i];
    tone(buzzPin, melody[i], Length);
    delay(Length * pause_flex + pause_fest);
    noTone(buzzPin);
  }
}

void loop() {

}
