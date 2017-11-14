#include "dht.h"
#define DHTin 9
#define DHTout 8

dht DHT;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("DHT Test");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
  int check = DHT.read22(DHTin);
  switch (check) {
    case DHTLIB_OK:
      Serial.print("Messung OK,\t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.print("Time out error,\t");
      break;
    default:
      Serial.print("Unknown error,\t");
      break;
  }
  Serial.print(DHT.temperature, 1);
  Serial.print(" °C\n");
  Serial.print(DHT.humidity, 1);
  Serial.print(" %\n");
}
