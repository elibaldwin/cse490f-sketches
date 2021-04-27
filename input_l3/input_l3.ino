
const int BTN_PIN = 2;
const int LED_PIN = 4;
const int DB_WIN = 10; // debounce window, in milliseconds

bool toggle = false;
int btnState = HIGH;

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

}

void loop() {
  int btnVal = digitalRead(BTN_PIN);
  delay(DB_WIN);
  int btnVal2 = digitalRead(BTN_PIN);

  if (btnVal == btnVal2 && btnVal != btnState) {
    btnState = btnVal;
    if (btnState == LOW) {
      toggle = !toggle;
    }
  }

  digitalWrite(LED_PIN, toggle);
}
