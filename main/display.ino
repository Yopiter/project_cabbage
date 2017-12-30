int Levelshit[][3] = {{6, 7}, {8, 9, 7}, {11, 12, 7} }; // manövrieren mit ner grenze der tiefe des ersten arrays, bei click gehts in die untere ebene und man weiß fürs zurück noch welches das oberstück ist.
//7 wäre der zurück text

char descriptions[][17] = {"" }; //Liste aller Texte in der ersten Zeile-> Stelle im Array = id in anderen arrays / maps

int topPos = 0;
bool second = false;
int secPos = 0;
bool third = false;
long standardTimeouts[2] = {10000, 60000};
long idleTime = 0;

int custBots[][2] = {{7, 0}, {10, 1}, {14, 2}, {16, 3}, {17, 4}};

int functioncall () { //Beipielfunktion für Messungen
  return 2;
}

void getBottom(int id, char botBuffer[17]) {
  char standardBottom[17] = " <   select   > "; //wenn custBot.find(id) nicht true oder so, dann das hier nehmen
  char thirdLayerBottom[17] = " -   select   + "; //Standard-Text für 3. Ebene Zeile 2
  char output[17];
  switch (id)
  {
    case 0: //temperatur
      sprintf(output, " <   %2.1d°C   > ", functioncall());
      break;
    case 1: //moisture mittelwert
      sprintf(output, " <     %3d    > ", functioncall());
      break;
    case 2: //letzte bewässerung mittelwert
      sprintf(output, " <  %4d min  > ", functioncall());
      break;
    case 3: //letzte düngung
      sprintf(output, " <   %3d h    > ", functioncall());
      break;
    case 4: //verbleibendes Licht
      sprintf(output, " <   %3d h    > ", functioncall());
      break;
    case 5:
      copy(standardBottom, output, sizeof(output));
    default:
      copy(thirdLayerBottom, output, sizeof(output));
      break;
  }
  copy(output, botBuffer, sizeof(output));
}

bool EngageUserMode_old() {
  if ((third && idleTime > standardTimeouts[1]) | (!third && idleTime > standardTimeouts[0])) {
    userMode = false; //Timeout: UserMode beenden und das auch der Loop-funktion sagen
    return false;
  }
  //Display
  if (!third)
  {

    //First Level Display is always the standard bot text
    if (!second)
    {
      char botText[17];
      getBottom(5, botText);
      PrintShit(descriptions[topPos], botText); //TODO Testen
    }

    //Second Level Displays need to look for special cases (Status messages)
    else
    {
      int descPos = Levelshit[topPos][secPos];
      int specialCase = -1;
      for (int cnt = 0; cnt < (sizeof(custBots) / sizeof(custBots[0])); cnt++) { //TODO gibts hier nen Stackoverflow?
        if (custBots[cnt][0] == descPos)
        {
          specialCase = cnt;
          break;
        }
      }

      char botText[17];
      if (specialCase >= 0)
        getBottom(custBots[specialCase][1], botText);
      else
        getBottom(5, botText);

      PrintShit(descriptions[descPos], botText);
    }
  }
  //Third Layer Display shit, still need to figure this out
  else
  {
    // Man könnte die betreffenden Variablen in ein Array "clonen" dass den Zahlen der SwitchCases gleicht -> hier die idleTime Mod 2000 und dann ein True / false senden -> gibt den Wechsel
    char botText[17];
    getBottom(911, botText);
    PrintShit(botText , botText); //TODO provisorischer topText -> Lösung finden
  }


  //Interaction logic
  int button = readButtons();

  if (button == -1) {
    idleTime += DELAY_USER_MODE;
    return false;
  }
  else {
    idleTime = 0; //Button pressed -> Idle Timer reset

    // Select gedrückt
    if (button == 1) {
      if (!second) { //Do first LAyer stuff -> entering second layer
        second = true;
      }
      else if (!third) { // Do second level Possibilities
        if (Levelshit[topPos][secPos] == 7) { //go back
          second = false;
          secPos = 0;
        }
        else {
          int descPos = Levelshit[topPos][secPos];
          bool special = false;
          for (int cnt = 0; cnt < (sizeof(custBots) / sizeof(custBots[0])); cnt++) { //TODO gibts hier nen Stackoverflow?
            if (custBots[cnt][0] == descPos)
            {
              special = true;
              break;
            }
          }

          //Do nothing when special (observing value) and go to setting Values (3rd layer) the rest of the time
          if (!special) {
            //TODO third level magic
            third = true;
          }
        }
      }
      else {
        //TODO set value
        third = false;
      }
    }

    //left gedrückt
    if (button == 0) {
      if (!second) {
        topPos -= 1;
        topPos = (topPos == -1) ? 4 : topPos; //TODO aufpassen, dass es angepasst wird, bei änderungen von Levelshit
      }
      else if (!third) {
        secPos -= 1;
        secPos = (secPos == -1) ? (sizeof(Levelshit[topPos]) / sizeof(Levelshit[topPos][0])) - 1 : secPos;
      }
      else {
        //TODO third level Magic

      }
    }

    //right gedrückt
    if (button == 2) {
      if (!second) {
        topPos += 1;
        topPos = (topPos >= 5) ? 0 : topPos; //TODO aufpassen, dass es angepasst wird, bei änderungen von Levelshit
      }
      else if (!third) {
        secPos += 1;
        secPos = (secPos >= sizeof(Levelshit[topPos]) / sizeof(Levelshit[topPos][0])) ? 0 : secPos;
      }
      else {
        //TODO third level Magic

      }
    }
    return true;
  }
}

void PrintShit(char * topText, char * botText) {

}

