#include <neotimer.h>

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
//1 Pin für Heizelement
#define HEIZUNG 5
////Shift Register End

//Pin für Ventilator //Ebenfalls über Relais/Transistor mit eigener Versorgung //Benötigt PWM-fähigen Digital-Pin
#define VentPin 3

//Pin für Relais zu Lampe
#define LichtPin 13

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
#define MIN_MOISTURE 600 //Maximaler Widerstandswert der Hygrometer -> minimaler Feuchte-Zustand des Bodens
#define MAX_MOISTURE 100 //Minimnaler Widerstandswert, dann Warnung
#define PUMP_TIME 5000 //Zeit, in der die Pumpen arbeiten, wenn der boden trocken ist. //TODO: Testen und besseren Wert finden!!
#define MAX_TEMP 30 //Höher werden wir ohne Heizelement niemals kommen... Keine Aloha-Temperaturen hier
#define MIN_TEMP 20 //Wir können nicht kühlen, also ist für ein Indoor-Konstrukt alles niedrigere eh unrealistisch
#define LEGIT_TEMP_DIFF 4 //Maximale Temp-Diff zwischen Sollwert und Avg. Temp. im Gehäuse
#define VAR_TEMP 2 //Temperaturgradient innerhalb des Gewächshauses
#define MAX_FERT_PUMP 10 //Maximale Pumpdauer in s für Düngerzugabe, minimal ist 0 für kein Dünger
#define INTERVAL_MOISTURE 100 //0,1 s warten zwischen Messungen der Bodenfeuchte
#define COOLDOWN_PUMPS 3 //3 Durchläufe warten nach dem Bewässern bis wieder gemessen wird, sonst eventuell Überwässerung
/////////////////

///////////////// Default Timeouts

#define TIMEOUT_USER_MODE 20000 //20 Sekunden Pause, bis sich der User-Mode beendet
#define DEF_LIGHTING 12 //12 h Beleuchtung täglich
#define DEF_FERT 168  //in h //1 mal pro Woche

/////////////////

///////////////// Konstanten

unsigned long TagesDauer = 24L * 60 * 60 * 1000;

/////////////////

///////////////// User-Variablen

dht DHT;
unsigned int moistures[4]; //Bodenfeuchte in % //Vielleicht lieber als Widerstandswert speichern?
unsigned int temperature;
unsigned int fertPumpTime;
unsigned long fertFreq; //in Millisekunden
unsigned long lichtDauer; //in Millisekunden

///////////////// Betriebsvariablen

int fanpower = 128; //Zwischen 0 und 255, wird mit analogWrite über PWM geschrieben und kontrolliert Lüfterstärke
int Hygros[4] = {Hyg0, Hyg1, Hyg2, Hyg3};
int MoisturePauseZyklen[4] = {0, 0, 0, 0};

///////////////// Timer

Neotimer LichtTimer;
Neotimer DunkelTimer;
Neotimer FertTimer;

///////////////// Prototypes

void writeToRegister(int PinToChange, int State = -1);
void DoPumpThings(int PumpenNummer, int Zustand = -1);
void ToggleHeizung(int Zustand = -1);

/////////////////


void setup() {
  initiatePins();
  Serial.begin(9600);
  Serial.println("Ich beschwöre den magischen Schnittlauch!!");
  initStandardValues();
}

void loop() {
  delay(DELAY_NORMAL);
  handleBodenFeuchten();
}

void initStandardValues() {
  moistures[0] = moistures[1] = moistures[2] = moistures[3] = 10; //I am ashamed of this...
  temperature = 22;
  fertPumpTime = 5;
  fertFreq = DEF_FERT * 60 * 60L;
  lichtDauer = DEF_LIGHTING * 60 * 60L;


  LichtTimer = Neotimer(lichtDauer);
  DunkelTimer = Neotimer(TagesDauer - lichtDauer);
  FertTimer = Neotimer(fertFreq);
}

int sekundenZuZyklen(int Sekunden) {
  return (int) Sekunden / DELAY_NORMAL * 1000;
}

int getTemperatur(int pin) {
  int check = DHT.read22(pin);
  char Nachricht[6];
  switch (check) {
    case DHTLIB_OK:
      //Eventuell grüne Lampe für Erfolg, aber eher nicht, um Strom zu sparen
      break;
    case DHTLIB_ERROR_CHECKSUM:
      sprintf(Nachricht, "Pin %i", pin);
      Fehler(DHT_CHECKSUM, Nachricht);
      break;
    case DHTLIB_ERROR_TIMEOUT:
      sprintf(Nachricht, "Pin %i", pin);
      Fehler(DHT_TIMEOUT, Nachricht);
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
    char Nachricht[30];
    sprintf(Nachricht, "Pin %i: %d", pin, sensorResistance);
    Fehler(MOISTURE_WARNING, Nachricht);
  }
  return ResistanceToMoisture(sensorResistance);
}

bool handleTemperaturen(int tIn, int tOut) {
  //Temperaturdurchschnitt sollte bei Solltemperatur liegen. Darum heißt sie Solltemperatur.
  int Abweichung = (tIn + tOut) / 2 - temperature;
  //Durchschnittstemperatur auswerten
  if (Abweichung < -VAR_TEMP) {
    //Insgesamt zu kalt im Gewächshaus -> Weniger Lüftung, damit mehr Zeit zum Erwärmen
    changeFanPower(-10);
    ToggleHeizung(HIGH);
  }
  if (Abweichung > VAR_TEMP) {
    //Zu warm im Gewächshaus -> mehr (hoffentlich kältere) Luft durchpusten
    changeFanPower(10);
    ToggleHeizung(LOW);
  }
  //Temperaturdifferenz auswerten
  //Temperaturgradient ist meiner Meinung nach gefährlicher als eine abweichende Gesamttemperatur, deshalb größere Änderungen, um sich im Konfliktfall durchzusetzen.
  if (tOut - tIn > LEGIT_TEMP_DIFF) {
    //Ziemlich hoher Temperaturgradient, spricht für schlechte Durchlüftung
    changeFanPower(20);
    if (Abweichung < -VAR_TEMP ) {
      //Zu kalt, aber gleichzeitig schlecht durchlüftet -> Dilemma :O
      //Heizung sollte bereits oben auf HIGH gestellt worden sein
      return false;
    }
  }
  return true;
}

void handleBodenFeuchten() {
  int i = 0;
  if (MoisturePauseZyklen[i] > 0) {
    //Nach dem Gießen ein kurzer Cooldown, damit sich das Wasser verteilen kann
    MoisturePauseZyklen[i]--;
    char CooldownNachricht[] = "Cooldown";
    Anzeigen(CooldownNachricht);
    return;
  }
  int Feuchte = getMoisture(Hygros[i]);
  char Nachricht[] = "Moisture d:  50";
  sprintf(Nachricht, "Moisture %d: %3d", i, Feuchte);
  Anzeigen(Nachricht);
  if (Feuchte < moistures[i] || Feuchte <= 0) {
    //Pumpen für den Abschnitt aktivieren
    DoPumpThings(i, 1);
    delay(PUMP_TIME); //Warten, während die Pumpen arbeiten
    DoPumpThings(i, 0);
    MoisturePauseZyklen[i] = COOLDOWN_PUMPS;
  }
}

void initiatePins() {
  pinMode(LatchPin, OUTPUT);
  pinMode(ClockPin, OUTPUT);
  pinMode(DataPin, OUTPUT);
  writeToRegister(0, LOW); //Shift Register komplett auf LOW stellen
}

void changeFanPower(int Differenz) {
  fanpower += Differenz;
  fanpower = constrain(fanpower, 0, 255);
  analogWrite(VentPin, fanpower);
}

int ResistanceToMoisture(int sensorResistance) {
  sensorResistance = constrain(sensorResistance, MAX_MOISTURE, MIN_MOISTURE);
  return map(sensorResistance, MAX_MOISTURE, MIN_MOISTURE, 100, 0);
}

int millisToHours(long mSec) {
  return (int)(mSec / 3600000);
}

void handleFertilizer() {
  if (FertTimer.done()) {
    char Nachricht[] = "fertTimer elapsed. Start fertilizing";
    Anzeigen(Nachricht);
    //TODO: Duenger zugeben
    DoFertPumpThings(HIGH);
    delay(fertPumpTime);
    DoFertPumpThings(LOW);
    FertTimer.start();
  }
}

void handleBeleuchtung() {
  if (!LichtTimer.started() && !DunkelTimer.started()) {
    //Initialisieren
    DoLichtThings(HIGH);
    LichtTimer.start();
    return;
  }
  if (LichtTimer.done()) {
    LichtTimer.reset();
    DoLichtThings(LOW);
    DunkelTimer.start();
    return;
  }
  if (DunkelTimer.done()) {
    DunkelTimer.reset();
    DoLichtThings(HIGH);
    LichtTimer.start();
    return;
  }
}

//Kehrt Input um, weil das Relais sich komisch verhält oO
void DoLichtThings(int Zustand) {
  constrain(Zustand, 0, 1);
  //TODO: Testen!! Das Relais scheint bei LOW zu ziehen?!
  //Vielleicht Relais-spezifisch? Das wäre uncool...
  Zustand = -Zustand + 1;
  digitalWrite(LichtPin, Zustand);
}

void Anzeigen(char msg[]) {
  Serial.println(msg);
}

