// this is a variation on the basic LED fade from https://makeabilitylab.github.io/physcomp/arduino/led-fade.html

const int LED_OUTPUT_PIN = 3;
const int MAX_ANALOG_OUT = 255; // the max analog output on the Uno is 255
const float DIVISOR = 720.0;

// The setup function runs once when you press reset or power the board
void setup() {
  // set the LED pin to as an output
  pinMode(LED_OUTPUT_PIN, OUTPUT);
  Serial.begin(9600); // for using Serial.println
}

// The loop function runs over and over again forever
void loop() {
  int brightness = int((sin(double(millis()) / DIVISOR) + 1) * (MAX_ANALOG_OUT / 2.0));
  Serial.println(brightness);
  
  // set the brightness of the LED pin
  analogWrite(LED_OUTPUT_PIN, brightness);
}
