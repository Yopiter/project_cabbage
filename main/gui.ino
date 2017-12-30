int Pfade[][5][3] = {{{1, 2}, {666}}, //Temeparature
  {{3, 4}, {5, 6}, {666}},            //Soil
  {{7, 8}, {666}},                    //Lighting
  {{9, 10}, {11, 12}, {666}},         //Fertilizer
  {{13}, {14}, {15}}                  //Status
};
char MenuePunkte[][17] = {"Temperature", "Soil", "Lighting", "Fertilizer", "Status"};

char SelectString[17] = " <   Select   > ";
char InDecreaseString[17] = " -            + ";


char TopLine[17] = "";
char BottomLine[17] = "";

int aktTiefe = 0;
int aktMenue = 0;
int aktUnterMenue = 0;

bool EngageUserMode() {

}

void setTopLine() {
  switch (getFormID()) {
    case (0):
      copy(MenuePunkte[aktMenue], TopLine, sizeof(MenuePunkte[aktMenue]));
      break;
    case (1):                           //Temperaturen
    case (2):
      strcpy(TopLine, "Target Temp.");
      break;
    case (3):
    case (4):
      strcpy(TopLine, "Min. Moisture");
      break;
    case (5):
    case (6):
      strcpy(TopLine, "Max. Moisture");
      break;
    case (7):
    case (8):
      strcpy(TopLine, "Lighting Time");
      break;
    case (9):
    case (10):
      strcpy(TopLine, "Fert. Intervall");
      break;
    case (11):
    case (12):
      strcpy(TopLine, "Fert. Volume");
      break;
    case (13):
      strcpy(TopLine, "Avg. Temeprature");
      break;
    case (14):
      strcpy(TopLine, "Avg. Moisture");
      break;
    case (15):
      strcpy(TopLine, "Last Fertilizer");
      break;
    case (16):
      strcpy(TopLine, "Remaining Light");
      break;
    case (666):
      strcpy(TopLine, "Back");
      break;
  }
}

void setBottomLine() {
  int FormID = getFormID();
  switch (FormID) {
    case (0):
    case (666):
      copy(SelectString, BottomLine, sizeof(SelectString));
      break;
    case (1):
      sprintf(BottomLine, " <    %i °C  > ", temperature);
      break;
    case (2): {
        printInDecreaseString(2, temperature, "C", 1);
        break;
      }
    case (3):                                               //Wie wird das mit den 4 verschiedenen Moistures gelöst?
      sprintf(BottomLine, " <    %i %   > ", moistures[0]); //TODO!!
      break;
    case (4):
      printInDecreaseString(2, moistures[0], "%", 1);
      break;
    case (5):
      break;
  }
}

int getFormID() {
  return aktTiefe != 0 ? Pfade[aktMenue][aktUnterMenue][aktTiefe] : 0;
}

void copy(char* src, char* dst, int len) {
  strncpy(dst, src, len);
}

//Erstellen des klassischen Strings zum Erhöhen/Senken von Werten. Aktualisiert BottomLine.
void printInDecreaseString(int Stellen, int Wert, const char *Einheit, int Einheitenlength) { //No prototype, weil gui.ino vor main.ino eingefügt wird
  int firstPart = (int)(7 - round(Stellen / 2 + 0.5)); //Letzter Index mit Leerzeichen
  char Zeile[17] = " -";
  whitespacen(Zeile, 2, firstPart - 1); //Initialisieren und mit Whitespaces füllen
  char Anzeige[Stellen + 1];
  aufStellenAuffuellen(Wert, Stellen, Anzeige);
  strcat(Zeile, Anzeige); //Wert anhängen
  int letzteStelle = firstPart + Stellen + 1;
  Zeile[letzteStelle] = ' ';
  letzteStelle++;
  for (int i = 0; i < Einheitenlength; i++) { //Einheit anhängen
    Zeile[letzteStelle + i] = Einheit[i];
  }
  letzteStelle += Einheitenlength;
  whitespacen(Zeile, letzteStelle, 14 - letzteStelle);
  strcat(Zeile, "+ ");
  copy(Zeile, BottomLine, sizeof(Zeile) / (sizeof(char)));
}

//Usage: char-Array mit Stellen+1 Elementen initialisieren und an Funktion übergeben
void aufStellenAuffuellen(int Wert, int Stellen, char* newString) {
  int IntLength = countDigits(Wert);
  if (Stellen < IntLength) { //Zu lang für Anzeigeplatz: Letzte Stellen werden abgeschnitten!
    char Nachricht[40];
    sprintf(Nachricht, "aufStellenAuf., %i auf %i Stellen", Wert, Stellen);
    Fehler(SYS_ERROR, Nachricht);
  }
  itoa(Wert, newString, 10);
  for (int i = IntLength; i < Stellen; i++) {
    newString[i] = ' ';
  }
  newString[Stellen] = '\0';
}

int countDigits(int num) {
  int count = 0;
  while (num) {
    num = num / 10;
    count++;
  }
  return count;
}

void whitespacen(char* Str, int Start, int len, bool terminieren) {
  for (int i = Start; i < Start + len; i++) {
    Str[i] = ' ';
  }
  if (terminieren) {
    Str[Start + len] = '\0';
  }
}

//Wahrscheinlich nicht mehr nötig
bool IntInArray(int Value, int* Array, int Arrlength) {
  for (int i = 0; i < Arrlength; i++) {
    if (Array[i] == Value) {
      return true;
    }
  }
  return false;
}


