// heavily based on https://makeabilitylab.github.io/physcomp/arduino/led-blink.html

const int LED_OUT_PIN = 3;
const int INTERVAL = 1000;

unsigned long _lastTimestep = 0;
int _ledState = LOW;

void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);
  digitalWrite(LED_OUT_PIN, HIGH);
  Serial.begin(9600);
}

void loop() {
  unsigned long timestep = millis();

  if (timestep - _lastTimestep >= INTERVAL) {
    _ledState = _ledState == HIGH ? LOW : HIGH;
    _lastTimestep = timestep;
    digitalWrite(LED_OUT_PIN, _ledState);
  }
}
