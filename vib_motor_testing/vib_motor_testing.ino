const int VIB_PIN = 5;
const int POT_PIN = A0;


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  int pot_val = analogRead(POT_PIN);
  analogWrite(VIB_PIN, map(pot_val, 0, 1023, 0, 255));
}
