const int RGB_R_PIN = 3;
const int RGB_G_PIN = 5;
const int RGB_B_PIN = 6;

const int DELAY_MS = 1000;

void setup() {
  pinMode(RGB_R_PIN, OUTPUT);
  pinMode(RGB_G_PIN, OUTPUT);
  pinMode(RGB_B_PIN, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  setRGBColor(LOW, LOW, LOW);  // off
  delay(DELAY_MS);
  
  setRGBColor(HIGH, LOW, LOW); // red
  delay(DELAY_MS);

  setRGBColor(LOW, HIGH, LOW); // green
  delay(DELAY_MS);

  setRGBColor(LOW, LOW, HIGH); // blue
  delay(DELAY_MS);

  setRGBColor(HIGH, LOW, HIGH);// purple
  delay(DELAY_MS);

  setRGBColor(LOW, HIGH, HIGH);// cyan
  delay(DELAY_MS);

  setRGBColor(HIGH, HIGH, LOW);// yellow
  delay(DELAY_MS);

  setRGBColor(HIGH, HIGH, HIGH);//white
  delay(DELAY_MS);
}

void setRGBColor(int r, int g, int b) {
  digitalWrite(RGB_R_PIN, r);
  digitalWrite(RGB_G_PIN, g);
  digitalWrite(RGB_B_PIN, b);
}
