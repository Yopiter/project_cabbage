#include "dht.h"

/////////////////Festlegen der Pinbelegung für die Signal-Leitungen
//Ich habe mich an den gelb-orangen Numemrn im Pinout (siehe Bild) orientiert, in der der Pin links unten die Nummer 9 hat.
//Pins für Temperatur/Luftfeuchte (DHT22) //2 Sensoren gesamt
#define DHTin 9 //DHT22 am Eingang (Luefter)
#define DHTout 8 //DHT22 am Ausgang

//Pins für Bodenfeuchte //4 Bodenfeuchte-Sensoren für 4 separate Bereiche //Benötigt analogen Eingang
//Kurze Beschreibung des Sensors: http://blog.whatgeek.com.pt/2014/08/arduino-hygrometer-or-humidity-sensor/
#define Hyg0 A0 //analog-Pin A0
#define Hyg1 A1
#define Hyg2 A2
#define Hyg3 A3

//Pins für Pumpen //4 Pumpen zur Bewässerung //Magnetventile wären cooler, kostet aber mehr als 2~3 kleine Pumpen...
//Pumpen werden entweder über Relay oder über Transistor angesteuert, haben eine eigene externe Stromversorgung mit 12 V
#define Pump0 10 //Pumpe für Gebiet 0
#define Pump1 11
#define Pump2 12
#define Pump3 13 //Eventuell hängt da die Board-LED dran, das könnte ein bisschen nerven

//Pin für eine eventuelle Düngerpumpe, die das Frischwasser mit Dünger spiken würde
#define PumdFert 2

//Pin für Ventilator //Ebenfalls über Relais/Transistor mit eigener Versorgung //Benötigt PWM-fähigen Digital-Pin
#define Vent 3

//Pins für Buttons
#define ButtonLinks 4
#define ButtonMitte 5
#define ButtonRechts 6
/////////////////
//Zusammengefasst: 4 analoge Pins und 10 digitale Pins belegt. 4 analoge und 4 digitale Pins noch frei. (Wenn man die Spezialpins mal ignoriert)
//Bei Bedarf für kleinere Ausgabereihen (Status-LEDs etc.) kann mittels Shift-Register erweitert werden (Habe ich noch welche da)
//Eventuell noch nötig:
//Pins für ein LED-Display (Benötigt 6 Pins)
//Pin für eine Pumpe, die das Wasserbecken wieder auffüllt
//Pins für Inputs (Potentiometer, Schalter oder Buttons)
//Pin für Füllstandsmessung des Wasserbeckens
//Pin für ein Heizelement
//Pins für Status-LEDs
//Pins für nerviges Piepen und Summen
/////////////////

///////////////// Fehlerzustände
#define DHT_TIMEOUT 0
#define DHT_CHECKSUM 1
#define MOISTURE_WARNING 2
/////////////////

///////////////// Parameter
#define DELAY_NORMAL 2000 //Wartezeit zwischen Zyklen
#define LEGIT_TEMP_DIFF 4 //Maximale Temp-Diff zwischen Ein- und Ausgang
#define MIN_MOISTURE 700 //Maximaler Widerstandswert der Hygrometer -> minimaler Feuchte-Zustand des Bodens
#define MAX_MOISTURE 280 //Minimnaler Widerstandswert, dann Warnung
#define MAX_TEMP 30
#define MIN_TEMP 20 //Wir können nicht kühlen, also ist für ein Indoor-Konstrukt alles niedrigere eh unrealistisch
/////////////////

int ButtonPins[3] = {ButtonLinks, ButtonMitte, ButtonRechts};

/////////////////

dht DHT;
unsigned int moistures[4]; //Bodenfeuchte in % //Vielleicht lieber als Widerstandswert speichern?
unsigned int temperature;
unsigned int fertPumpTime;
unsigned long fertFreq; //in Sekunden
unsigned int fertZyklenRemaining;
unsigned long lichtDauer; //in Sekunden
unsigned int lichtZyklenRemaining;

/////////////////


void setup() {
  initiateButtons();
  Serial.begin(9600);
  Serial.println("Ich beschwöre den magischen Schnittlauch!!");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(DELAY_NORMAL);
  int in = getTemperatur(DHTin);
  int out = getTemperatur(DHTout);
  //TODO: Prüfung der Temperaturunterschiede und der Durchschnittstemperatur
  //Dann Regelung des Lüfters über PWM
  //Wenn gegeben auch Heizelement regeln
  //Dann Prüfung der Bodenfeuchten und bei Bedarf betätigen der Pumpen
  //Prüfung der Feuchte eventuell auch nicht bei jedem Zyklus
  //GROßES TODO: Planen, wie man die Bedienung realisieren könnte:
  //Nötig sind Eingaben für Temperatur, Belichtungszeit, 4 x Bodenfeuchtewerte und eventuell für Düngerzugabe
}

bool initStandardValues() {
  moistures[0] = moistures[1] = moistures[2] = moistures[3] = 10; //I am ashamed of this...
  temperature = 22;
  fertPumpTime = 10;
  fertFreq = 7 * 24 * 60 * 60L;
  fertZyklenRemaining = sekundenZuZyklen(fertFreq);
  lichtDauer = 12 * 60 * 60L;
  lichtZyklenRemaining = sekundenZuZyklen(lichtDauer);
}

int sekundenZuZyklen(int Sekunden) {
  return (int) Sekunden / DELAY_NORMAL * 1000;
}

int getTemperatur(int pin) {
  int check = DHT.read22(pin);
  switch (check) {
    case DHTLIB_OK:
      //Eventuell grüne Lampe für Erfolg, aber eher nicht, um Strom zu sparen
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Fehler(DHT_TIMEOUT, "Pin " + pin); //Not implemented yet
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Fehler(DHT_TIMEOUT, "Pin " + pin);
      break;
    default:
      Serial.print("Unknown error,\t");
      break;
  }
  //Nur zum Debuggen, später raus oder auf LCD-Screen
  Serial.print("Pin " + pin);
  Serial.print(DHT.temperature, 1);
  Serial.print(" °C\t");
  Serial.print(DHT.humidity, 1);
  Serial.print(" %\t");
  //Bis hier hin siehe oben
  return DHT.temperature;
}

bool Fehler(int Identifier, String meldung) {
  switch (Identifier) {
    case (DHT_TIMEOUT):
      char Nachricht[25];
      sprintf(Nachricht, "DHT-Sensor Timed out at %d: ", millis());
      Anzeigen(Nachricht + meldung);
      break;
    case (MOISTURE_WARNING):
      break;
    case (DHT_CHECKSUM):
      break;
    //TODO: Für weitere Fehlerfälle anpassen
    default:
      Serial.println("unbekannter Fehler: " + Identifier);
      Serial.println(meldung);
      break;
  }
}

int getMoisture(int pin) {
  int sensorResistance = analogRead(pin);
  if (sensorResistance < MAX_MOISTURE || sensorResistance > MIN_MOISTURE) {
    Fehler(MOISTURE_WARNING, "Pin " + pin);
  }
  sensorResistance = constrain(sensorResistance, MAX_MOISTURE, MIN_MOISTURE);
  return map(sensorResistance, MAX_MOISTURE, MIN_MOISTURE, 100, 0);
}

void initiateButtons() {
  for (int i = 0; i < sizeof(ButtonPins) / sizeof(int); i++) {
    pinMode(ButtonPins[i], INPUT);
  }
}

int readButtons() {
  for (int i = 0; i < sizeof(ButtonPins) / sizeof(int); i++) {
    if (digitalRead(ButtonPins[i]) == HIGH) return i;
  }
  return -1;
  
bool tempInRange(int Temp) {
  return Temp < MAX_TEMP && Temp > MIN_TEMP;
}

