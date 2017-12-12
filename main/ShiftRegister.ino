byte ShiftRegisterZustand = 0;

void DoPumpThings(int PumpenNummer, int Zustand) { //Bloß Umwandlung von Pumpennummer in Pinnummer des Schift Registers
  writeToRegister(PumpenNummer + 1, Zustand);
}

void ToggleHeizung(int Zustand) {
  writeToRegister(Heizung, Zustand);
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
}
