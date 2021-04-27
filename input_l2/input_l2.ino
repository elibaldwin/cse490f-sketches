#define KEY_C 262  // 261.6256 Hz (middle C)
#define KEY_D 294  // 293.6648 Hz
#define KEY_E 330  // 329.6276 Hz
#define KEY_F 350  // 349.2282 Hz
#define KEY_G 392  // 391.9954 Hz

const int INPUT_BUTTON_C_PIN = 2;
const int INPUT_BUTTON_D_PIN = 3;
const int INPUT_BUTTON_E_PIN = 7;
const int INPUT_BUTTON_F_PIN = 11;
const int INPUT_BUTTON_G_PIN = 12;

const int OUTPUT_PIEZO_PIN = 9; // hook up one piezo leg to GND, other leg to Pin 9
const int OUTPUT_LED_PIN = LED_BUILTIN; // we'll flash an LED on/off with the sound
const int SOUND_DURATION_MS = 500; // duration to play each siren part

void setup() {
  pinMode(INPUT_BUTTON_C_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_D_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_E_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_F_PIN, INPUT_PULLUP);
  pinMode(INPUT_BUTTON_G_PIN, INPUT_PULLUP);
  pinMode(OUTPUT_PIEZO_PIN, OUTPUT);
  pinMode(OUTPUT_LED_PIN, OUTPUT);
}

void loop() {
  if (btnDown(INPUT_BUTTON_C_PIN)) {
    tone(OUTPUT_PIEZO_PIN, KEY_C);
  } else if (btnDown(INPUT_BUTTON_D_PIN)) {
    tone(OUTPUT_PIEZO_PIN, KEY_D);
  } else if (btnDown(INPUT_BUTTON_E_PIN)) {
    tone(OUTPUT_PIEZO_PIN, KEY_E);
  } else if (btnDown(INPUT_BUTTON_F_PIN)) {
    tone(OUTPUT_PIEZO_PIN, KEY_F);
  } else if (btnDown(INPUT_BUTTON_G_PIN)) {
    tone(OUTPUT_PIEZO_PIN, KEY_G);
  } else {
    noTone(OUTPUT_PIEZO_PIN);
    digitalWrite(OUTPUT_LED_PIN, LOW);
  }
}

boolean btnDown(int btnPin) {
  int state = digitalRead(btnPin);
  digitalWrite(OUTPUT_LED_PIN, !state);
  return state == LOW;
}
