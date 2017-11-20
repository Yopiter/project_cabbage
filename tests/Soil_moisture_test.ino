#define MOISTURE_PIN A7
void setup() {
  Serial.begin(9600); //Messung durch Druck auf Reset
  int resistance = analogRead(MOISTURE_PIN);
  Serial.print(resistance);
}

void loop() {
  // put your main code here, to run repeatedly:
}
