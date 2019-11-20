#include <neotimer.h>

#include "dht.h"

/////////////////Festlegen der Pinbelegung für die Signal-Leitungen

//Pins für Bodenfeuchte //4 Bodenfeuchte-Sensoren für 4 separate Bereiche //Benötigt analogen Eingang
//Kurze Beschreibung des Sensors: http://blog.whatgeek.com.pt/2014/08/arduino-hygrometer-or-humidity-sensor/
#define Hyg0 A0 //analog-Pin A0
#define Hyg1 A1
#define Hyg2 A2
#define Hyg3 A3

//Pins für Shift Register
#define ClockPin 10
#define LatchPin 11
#define DataPin 12

////Pins auf Shift-Register
//4 Pumpen zur Bewässerung, 1 Pumpe für Dünger
//Pumpen werden entweder über Relay oder über Transistor angesteuert, haben eine eigene externe Stromversorgung mit 12 V
#define PUMP_FERT 0
#define PUMP0 1 //Pumpe für Gebiet 0
#define PUMP1 2
#define PUMP2 3
#define PUMP3 4
//Pumpen-Pins auf Mega 2560
#define PUMP_FERT_M 22
#define PUMP0_M 23 //Pumpe für Gebiet 0
#define PUMP1_M 24
#define PUMP2_M 25
#define PUMP3_M 26

#define PIN_WATER 27
////Shift Register End

///////////////// Fehlerzustände
#define DHT_TIMEOUT 0
#define DHT_CHECKSUM 1
#define MOISTURE_WARNING 2
#define TEMP_HANDLING_ERROR 3
#define SYS_ERROR 4
/////////////////

///////////////// Konstanten
#define PUMP_ON 0
#define PUMP_OFF 1
/////////////////

///////////////// Parameter
#define DELAY_NORMAL 2000 //Wartezeit zwischen Zyklen
#define MIN_MOISTURE 700 //Maximaler Widerstandswert der Hygrometer -> minimaler Feuchte-Zustand des Bodens
#define MAX_MOISTURE 230 //Minimnaler Widerstandswert, dann Warnung
#define PUMP_TIME 4 //Zeit, in der die Pumpen arbeiten, wenn der boden trocken ist. //TODO: Testen und besseren Wert finden!!

#define MAX_FERT_PUMP 10 //Maximale Pumpdauer in s für Düngerzugabe, minimal ist 0 für kein Dünger
#define INTERVAL_MOISTURE 100 //0,1 s warten zwischen Messungen der Bodenfeuchte
#define COOLDOWN_PUMPS 5 //x Durchläufe warten nach dem Bewässern bis wieder gemessen wird, sonst eventuell Überwässerung
/////////////////

///////////////// Default Timeouts

#define TIMEOUT_USER_MODE 20000 //20 Sekunden Pause, bis sich der User-Mode beendet
#define DEF_LIGHTING 12 //12 h Beleuchtung täglich
#define DEF_FERT 168  //in h //1 mal pro Woche

/////////////////

///////////////// User-Variablen

unsigned int moistures[4]; //Bodenfeuchte in % //Vielleicht lieber als Widerstandswert speichern?
unsigned int fertPumpTime;
unsigned long fertFreq; //in Millisekunden

///////////////// Betriebsvariablen

int Hygros[4] = {Hyg0, Hyg1, Hyg2, Hyg3};
int MoisturePauseZyklen[4] = {0, 0, 0, 0};
int PumpenPins[4] = {PUMP0_M, PUMP1_M, PUMP2_M, PUMP3_M};

///////////////// Timer

Neotimer FertTimer;

///////////////// Prototypes

void writeToRegister(int PinToChange, int State = -1);
void DoPumpThings(int PumpenNummer, int Zustand = -1);

/////////////////


void setup() {
  initiatePins();
  Serial.begin(9600);
  Serial.println("Ich beschwöre den magischen Schnittlauch!!");
  initStandardValues();
}

void loop() {
  if (!getWasserStand()) {
    //Füllstandsanzeige des Topfes sagt leer
    Serial.println("Wasser alle!");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(2000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(2000);
    return;
  }
  delay(DELAY_NORMAL);
  //Dann Prüfung der Bodenfeuchten und bei Bedarf betätigen der Pumpen
  handleBodenFeuchten();
  handleFertilizer();
}

void initStandardValues() {
  moistures[0] = moistures[1] = moistures[2] = moistures[3] = 30; //I am ashamed of this...
  fertPumpTime = 5;
  fertFreq = DEF_FERT * 60 * 60L;

  FertTimer = Neotimer(fertFreq);
}

void Fehler(int Identifier, char meldung[]) {
  char Nachricht[32];
  switch (Identifier) {
    case (DHT_TIMEOUT):
      sprintf(Nachricht, "DHT-Sensor Timed out at %d: ", millis());
      break;
    case (MOISTURE_WARNING):
      sprintf(Nachricht, "Moisture out of Bounds: ");
      break;
    case (TEMP_HANDLING_ERROR):
      sprintf(Nachricht, "Error when handling temp.: ");
      break;
    case (DHT_CHECKSUM):
      sprintf(Nachricht, "DHT Checksum Error: ");
      break;
    case (SYS_ERROR):
      sprintf(Nachricht, "System Error: ");
      break;
    //TODO: Für weitere Fehlerfälle anpassen
    default:
      Serial.println("unbekannter Fehler: " + Identifier);
      Serial.println(meldung);
      break;
  }
  Anzeigen(Nachricht);
  Anzeigen(meldung);
}

int getMoisture(int pin) {
  //Mittelwert aus 5 Messungen für Pseudo-Robustheit xD
  int sensorResistance = 0;
  for (int i = 0; i < 5; i++) {
    sensorResistance += analogRead(pin);
    delay(INTERVAL_MOISTURE);
  }
  sensorResistance = sensorResistance / 5;
  if (sensorResistance < MAX_MOISTURE || sensorResistance > MIN_MOISTURE) {
    char Nachricht[12];
    sprintf(Nachricht, "Pin %i: $i", pin, sensorResistance);
    Fehler(MOISTURE_WARNING, Nachricht);
  }
  return ResistanceToMoisture(sensorResistance);
}

void handleBodenFeuchten() {
  for (int i = 0; i < 4; i++) {
    if (MoisturePauseZyklen[i] > 0) {
      //Nach dem Gießen ein kurzer Cooldown, damit sich das Wasser verteilen kann
      MoisturePauseZyklen[i]--;
      continue;
    }
    int Feuchte = getMoisture(Hygros[i]);
    char Nachricht[] = "Moisture d:  50";
    sprintf(Nachricht, "Moisture %d: %3d", i, Feuchte);
    Anzeigen(Nachricht);
    if (Feuchte < moistures[i] || Feuchte <= 0) {
      //Pumpen für den Abschnitt aktivieren
      DoPumpThingsMega(i, PUMP_ON);
      delay(PUMP_TIME * 1000); //Warten, während die Pumpen arbeiten
      DoPumpThingsMega(i, PUMP_OFF);
      MoisturePauseZyklen[i] = COOLDOWN_PUMPS;
    }
  }
}

void initiatePins() {
  pinMode(LatchPin, OUTPUT);
  pinMode(ClockPin, OUTPUT);
  pinMode(DataPin, OUTPUT);
  writeToRegister(0, HIGH); //Shift Register komplett auf HIGH stellen

  for (int i = 0; i < 4; i++) {
    pinMode(PumpenPins[i], OUTPUT);
    digitalWrite(PumpenPins[i], HIGH);
  }

  pinMode(PIN_WATER, INPUT);
}

int ResistanceToMoisture(int sensorResistance) {
  sensorResistance = constrain(sensorResistance, MAX_MOISTURE, MIN_MOISTURE);
  return map(sensorResistance, MAX_MOISTURE, MIN_MOISTURE, 100, 0);
}

void handleFertilizer() {
  if (FertTimer.done()) {
    char Nachricht[] = "fertTimer elapsed. Start fertilizing";
    Anzeigen(Nachricht);
    //TODO: Duenger zugeben
    DoFertPumpThingsMega(PUMP_ON);
    delay(fertPumpTime);
    DoFertPumpThingsMega(PUMP_OFF);
    FertTimer.start();
  }
}

void DoPumpThingsMega(int pumpenNummer, int Zustand) {
  Serial.print(pumpenNummer);
  Serial.print(" ist jetzt ");
  Serial.print(Zustand);
  digitalWrite(PumpenPins[pumpenNummer], Zustand);
}

void DoFertPumpThingsMega(int Zustand) {
  digitalWrite(PUMP_FERT_M, Zustand);
}

bool getWasserStand() {
  if (digitalRead(PIN_WATER) == HIGH) {
    return false;
  }
  else {
    return true;
  }
}

void Anzeigen(char msg[]) {
  Serial.println(msg);
}

