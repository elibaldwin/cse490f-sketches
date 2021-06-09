#include <Servo.h>

#include <Ultrasonic.h>


const int G_LED = 5;
const int Y_LED = 10;
const int R_LED = 11;

Servo dial;
Ultrasonic dist(8, 7, 6000UL);

int dial_pos;

void setup() {
  Serial.begin(115200);
  dial.attach(9);

  pinMode(G_LED, OUTPUT);
  pinMode(Y_LED, OUTPUT);
  pinMode(R_LED, OUTPUT);
}

void loop() {
  int dist_read = dist.read();
  
  while (Serial.available() > 0) {
    String in_data = Serial.readStringUntil('\n');
    int health = in_data.toInt();
    setDial(health * 2);
  }

  Serial.println(dist_read);

  delay(100);
}


// accepts percent, from 0 to 100.
void setDial(int percent) {
  dial.write(map(percent, 0, 100, 180, 0));
  if (percent < 20) {
    digitalWrite(R_LED, HIGH);
    digitalWrite(Y_LED, LOW);
    digitalWrite(G_LED, LOW);
  } else if (percent < 70) {
    digitalWrite(R_LED, LOW);
    digitalWrite(Y_LED, HIGH);
    digitalWrite(G_LED, LOW);
  } else {
    digitalWrite(R_LED, LOW);
    digitalWrite(Y_LED, LOW);
    digitalWrite(G_LED, HIGH);
  }
}
