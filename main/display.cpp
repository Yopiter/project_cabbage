#include <stdio.h> //sprintf

using namespace std;

//Auslesefunktionen der Pins in ein Headerfile "Schnittstelle"

int Levelshit[][3] = {{6, 7}, {8, 9, 7}, {11, 12, 7} }; // manövrieren mit ner grenze der tiefe des ersten arrays, bei click gehts in die untere ebene und man weiß fürs zurück noch welches das oberstück ist.
//7 wäre der zurück text

char standardBottom[17] = " <   select   > "; //wenn custBot.find(id) nicht true oder so, dann das hier nehmen
char descriptions[][17] = {"" }; //Liste aller Texte in der ersten Zeile-> Stelle im Array = id in anderen arrays / maps
char thirdLayerBottom[17] = " -   select   + "; //Standard-Text für 3. Ebene Zeile 2
char specialBottom[17];

int custBots[][2] = {{7, 0}, {10, 1}, {14, 2}, {16, 3}, {17, 4}, {20, 5}, {21, 5}, {22, 5}};

int functioncall () { //Beipielfunktion für Messungen
  return 2;
}

void custBottom(int id) {
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
  }
  for (int cnt = 0; cnt <= sizeof(output) / sizeof(*output) - 1; cnt++)
  {
    specialBottom[cnt] = output[cnt];
  }

}

int main ()
{
  int topPos = 0;
  bool second = false;
  int secPos = 0;
  bool third = false;
}

