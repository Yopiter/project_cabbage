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

//Pins für Shift Register
#define DataPin 10
#define ClockPin 11
#define LatchPin 12

////Pins auf Shift-Register
//4 Pumpen zur Bewässerung, 1 Pumpe für Dünger
//Pumpen werden entweder über Relay oder über Transistor angesteuert, haben eine eigene externe Stromversorgung mit 12 V
#define Pump0 1 //Pumpe für Gebiet 0
#define Pump1 2
#define Pump2 3
#define Pump3 4
#define PumpFert 0
//1 Pin für Heizelement
#define Heizung 5
////Shift Register End

//Pin für Ventilator //Ebenfalls über Relais/Transistor mit eigener Versorgung //Benötigt PWM-fähigen Digital-Pin
#define VentPin 3

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
#define TEMP_HANDLING_ERROR 3
#define SYS_ERROR 4
/////////////////

///////////////// Parameter
#define DELAY_NORMAL 2000 //Wartezeit zwischen Zyklen
#define DELAY_USER_MODE 200 //Abfragetakt der Buttons und der Ausgabe
#define LEGIT_TEMP_DIFF 4 //Maximale Temp-Diff zwischen Ein- und Ausgang
#define MIN_MOISTURE 700 //Maximaler Widerstandswert der Hygrometer -> minimaler Feuchte-Zustand des Bodens
#define MAX_MOISTURE 280 //Minimnaler Widerstandswert, dann Warnung
#define PUMP_TIME 10 //Zeit, in der die Pumpen arbeiten, wenn der boden trocken ist. //TODO: Testen und besseren Wert finden!!
#define MAX_TEMP 30 //Höher werden wir ohne Heizelement niemals kommen... Keine Aloha-Temperaturen hier
#define MIN_TEMP 20 //Wir können nicht kühlen, also ist für ein Indoor-Konstrukt alles niedrigere eh unrealistisch
#define VAR_TEMP 2 //2°C Abweichung klingen gut, hoffe ich? Fuck this, wir brauchen einen scheiß Botaniker...
///////////////// Variablen für den Input

int ButtonPins[3] = {ButtonLinks, ButtonMitte, ButtonRechts};

///////////////// User-Variablen

dht DHT;
unsigned int moistures[4]; //Bodenfeuchte in % //Vielleicht lieber als Widerstandswert speichern?
unsigned int temperature;
unsigned int fertPumpTime;
unsigned long fertFreq; //in Sekunden
unsigned int fertZyklenRemaining;
unsigned long lichtDauer; //in Sekunden
unsigned int lichtZyklenRemaining;

///////////////// Betriebsvariablen

int fanpower = 128; //Zwischen 0 und 255, wird mit analogWrite über PWM geschrieben und kontrolliert Lüfterstärke
int Hygros[4] = {Hyg0, Hyg1, Hyg2, Hyg3};
bool userMode = false;

///////////////// Prototypes

void writeToRegister(int PinToChange, int State = -1);
void DoPumpThings(int PumpenNummer, int Zustand = -1);
void ToggleHeizung(int Zustand = -1);

/////////////////


void setup() {
  initiatePins();
  Serial.begin(9600);
  Serial.println("Ich beschwöre den magischen Schnittlauch!!");
}

void loop() {
  if (!userMode) {
    delay(DELAY_NORMAL);
    if (!handleTemperaturen(getTemperatur(DHTin), getTemperatur(DHTout))) {
      Fehler(TEMP_HANDLING_ERROR, "Zu kalt, aber auch schlecht durchlüftet!");
    }
    //Dann Prüfung der Bodenfeuchten und bei Bedarf betätigen der Pumpen
    handleBodenFeuchten();
    //GROßES TODO: Planen, wie man die Bedienung realisieren könnte:
    //Nötig sind Eingaben für Temperatur, Belichtungszeit, 4 x Bodenfeuchtewerte und eventuell für Düngerzugabe
  }
  else {
    delay(DELAY_USER_MODE);
    if (EngageUserMode()) {
      //Alles fit im Schritt, geht okidoki weiter
      //Vielleicht etwas Musik spielen
    }
    else {
      //Usermode wurde beendet
      //Display ausschalten
    }
    //Eventuelle Counter weiterlaufen lassen, aber nur weniger Zeit abziehen
    //Bei den Countern in Zyklen nur jedes zehnte Mal einen abziehen oder es einfach lassen, macht nix
  }
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
    case (TEMP_HANDLING_ERROR):
      break;
    case (DHT_CHECKSUM):
      break;
    case (SYS_ERROR):
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

bool handleTemperaturen(int tIn, int tOut) {
  //Temperaturdurchschnitt sollte bei Solltemperatur liegen. Darum heißt sie Solltemperatur.
  int Abweichung = (tIn + tOut) / 2 - temperature;
  //Durchschnittstemperatur auswerten
  //Wenn gegeben später auch Heizelement regeln
  if (Abweichung < -2) {
    //Insgesamt zu kalt im Gewächshaus -> Weniger Lüftung, damit mehr Zeit zum Erwärmen
    changeFanPower(-10);
    ToggleHeizung(HIGH);
  }
  if (Abweichung > 2) {
    //Zu warm im Gewächshaus -> mehr (hoffentlich kältere) Luft durchpusten
    changeFanPower(10);
    ToggleHeizung(LOW);
  }
  //Temperaturdifferenz auswerten
  //Temperaturgradient ist meiner Meinung nach gefährlicher als eine abweichende Gesamttemperatur, deshalb größere Änderungen, um sich im Konfliktfall durchzusetzen.
  if (tOut - tIn > LEGIT_TEMP_DIFF) {
    //Ziemlich hoher Temperaturgradient, spricht für schlechte Durchlüftung
    changeFanPower(20);
    if (Abweichung < -2 ) {
      //Zu kalt, aber gleichzeitig schlecht durchlüftet -> Dilemma :O
      //Heizung sollte bereits oben auf HIGH gestellt worden sein
      return false;
    }
  }
  return true;
}

void handleBodenFeuchten() {
  for (int i = 0; i < 4; i++) {
    int Feuchte = getMoisture(Hygros[i]);
    if (Feuchte < moistures[i] - 10 || Feuchte <= 0) {
      //Pumpen für den Abschnitt aktivieren
      DoPumpThings(i, 1);
      delay(PUMP_TIME * 1000); //Warten, während die Pumpen arbeiten //TODO: Hier eventuell alle aktuellen Timer weiter laufen lassen
      DoPumpThings(i, 0);
    }
  }
}

void initiatePins() {
  for (int i = 0; i < sizeof(ButtonPins) / sizeof(int); i++) {
    pinMode(ButtonPins[i], INPUT);
  }
  pinMode(LatchPin, OUTPUT);
  pinMode(ClockPin, OUTPUT);
  pinMode(DataPin, OUTPUT);
  writeToRegister(0, LOW); //Shift Register komplett auf LOW stellen
}

int readButtons() {
  for (int i = 0; i < sizeof(ButtonPins) / sizeof(int); i++) {
    if (digitalRead(ButtonPins[i]) == HIGH) return i;
  }
  return -1;
}

bool tempInputInRange(int Temp) { //Bei Eingabe prüfen, ob die Zielwerte realistisch sind
  return Temp < MAX_TEMP && Temp > MIN_TEMP;
}

void changeFanPower(int Differenz) {
  fanpower += Differenz;
  fanpower = constrain(fanpower, 0, 255);
  analogWrite(VentPin, fanpower);
}

bool Anzeigen(String Text) {
  //TODO: Entscheidung, ob über Serial oder über LCD
}

