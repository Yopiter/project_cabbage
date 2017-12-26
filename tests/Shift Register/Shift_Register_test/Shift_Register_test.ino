//Pins für Shift Register
#define DataPin 10
#define ClockPin 11
#define LatchPin 13

byte ShiftRegisterZustand = 0;

void writeToRegister(int PinToChange, int State = -1);

void setup() {
  pinMode(LatchPin, OUTPUT);
  pinMode(ClockPin, OUTPUT);
  pinMode(DataPin, OUTPUT);
  writeToRegister(0, LOW); //Alles auf LOW initialisieren
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
  Serial.println(PinToChange, State);
}

