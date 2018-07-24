byte ShiftRegisterZustand = 255;

void DoPumpThings(int PumpenNummer, int Zustand) { //Bloß Umwandlung von Pumpennummer in Pinnummer des Schift Registers
  constrain(PumpenNummer, 0, 3);  //Nur für Wasserpumpen (Pin 1-4)
  constrain(Zustand, 0, 1);
  writeToRegister(PumpenNummer + 1, Zustand);
}

void DoFertPumpThings(int Zustand) {
  constrain(Zustand, 0, 1);
  writeToRegister(PUMP_FERT, Zustand);
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
