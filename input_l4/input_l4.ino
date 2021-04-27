const int POT_PIN = A0;
const int LED_PIN = 3;
const int DELAY = 50;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int potVal = analogRead(POT_PIN);

  int brightness = map(potVal, 0, 1023, 0, 255);

  analogWrite(LED_PIN, brightness);
  delay(DELAY);
}
