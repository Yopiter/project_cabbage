
#include "dht.h"

#define TEMP_PIN 2
dht DHT;

void setup() {
  Serial.begin(9600);
  Serial.println("DHT-Test");
}

void loop() {
  getTemperatur(TEMP_PIN);
  delay(1000);
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
      Serial.println("DHT Checksum Error!!\n");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      sprintf(Nachricht, "Pin %i", pin);
      Serial.println("DHT Timeout!!\n");
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
