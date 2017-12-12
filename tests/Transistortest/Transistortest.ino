#define SigPin A1
void setup() {
  pinMode(SigPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(SigPin, HIGH);
  digitalWrite(13, HIGH);
  delay(2000);
  digitalWrite(SigPin, LOW);
  digitalWrite(13, LOW);
  delay(2000);
}
