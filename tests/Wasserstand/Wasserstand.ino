#define PIN_WATER A0

int Durchlauf = 0;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_WATER, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(analogRead(PIN_WATER));
  if (digitalRead(PIN_WATER) == LOW) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Kein Wasser");
  }
  else {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.println("Wasser");
  }
  Durchlauf++;
  delay(100);
}
