//Pins für Shift Register
#define ClockPin 10
#define LatchPin 11
#define DataPin 12

byte ShiftRegisterZustand = 255;

void writeToRegister(int PinToChange, int State = -1);

void setup() {
  pinMode(LatchPin, OUTPUT);
  pinMode(ClockPin, OUTPUT);
  pinMode(DataPin, OUTPUT);
  writeToRegister(0, HIGH); //Alles auf HIGH initialisieren
  Serial.begin(9600);
  Serial.println("reset");
}

void loop() {
  if (Serial.available() > 0) {
    int bitToSet = Serial.read() - 48;
    writeToRegister(bitToSet);
  }
}

void writeToRegister(int PinToChange, int State) {
  if (State == -1) { //Toggle
    State = bitRead(ShiftRegisterZustand, PinToChange) - 1;
    State = abs(State); //Auf eigener Zeile, weil es so in der Doku "empfohlen" wurde...
  }
  bitWrite(ShiftRegisterZustand, PinToChange, State);
  digitalWrite(LatchPin, LOW);
  shiftOut(DataPin, ClockPin, MSBFIRST, ShiftRegisterZustand);
  digitalWrite(LatchPin, HIGH);
  Serial.println(ShiftRegisterZustand);
}

