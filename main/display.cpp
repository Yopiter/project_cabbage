#include <string> //string, duh.
#include <stdio.h> //sprintf
using namespace std;

//Auslesefunktionen der Pins in ein Headerfile "Schnittstelle"

int Levelshit[][] = {{6,7}, {8,9,7}, {11,12,7} } // manövrieren mit ner grenze der tiefe des ersten arrays bei cklick gehts in die untere ebene und man weiß fürs zurück noch welches das oberstück ist. Für die 3. Ebene hab ich kp ehrlich :/ vlt wieder hart coden wie beim custBottom()
//7 wäre der zurück text

string standardBottom = " <   select   > "; //wenn custBot.find(id) nicht true oder so, dann das hier nehmen
string descriptions[x] = {"" }; //Liste aller oberen Texte -> Stelle im Array = id in anderen arrays / maps

map<int,int> custBot={{7,0},{10,1},{14,2},{16,3},{17,4},{20,5},{21,5},{22,5}}; 
string custBottom(int id) {
  string output;
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
    case 5: //- + texte
      output = " -   select   + ";
  }
  return output;
}
