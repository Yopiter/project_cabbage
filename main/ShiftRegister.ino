int pumpenZustand[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void WritePumpValues() {
  //Man weiß ja nie, wann mans mal vergessen hat auszumachen ^^
  digitalWrite(DataPin, LOW);
  digitalWrite(ClockPin, LOW);
  //GOGO!
  digitalWrite(LatchPin, LOW);
  for (int i = 0; i < 8; i++) {
    digitalWrite(ClockPin, LOW);
    digitalWrite(DataPin, pumpenZustand[i]);
    digitalWrite(ClockPin, HIGH);
    digitalWrite(DataPin, LOW);
  }
  digitalWrite(ClockPin, LOW);
  digitalWrite(LatchPin, HIGH);
}

void DoPumpThings(int PumpenNummer, int Zustand) { //wenn man einen Default-Wert für Zustand (-1) übergibt, kackt der Compiler ab XD
  switch (Zustand) {
    case (0):
    case (1):
      pumpenZustand[PumpenNummer] = Zustand;
      break;
    case (-1): //Toggle, auch wenn ich nicht glaube dass wirs jemals brauchen werden :D
      pumpenZustand[PumpenNummer] = pumpenZustand[PumpenNummer] == 0 ? 1 : 0;
  }
  WritePumpValues();
}
