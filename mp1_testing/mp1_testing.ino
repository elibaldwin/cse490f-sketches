#include "RingBuffer.h"
#include "src/RGBConverter/RGBConverter.h"
const boolean COMMON_ANODE = false; 

const int POT_PIN = A0;

// configuration values for calibrating potentiometer range
const int POT_MIN = 220; 
const int POT_MAX = 960;

const int LIGHTSENSE_PIN = A1;

// pins for the two parallel RGB LEDs
const int RGB_R_PIN = 6;
const int RGB_G_PIN = 5;
const int RGB_B_PIN = 3;

// pins for each pair of parallel white LEDs
const int WHT1_PIN = 9;
const int WHT2_PIN = 10;

// pin for the red button LED
const int BTN_LED_PIN = 11;

RingBuffer smooth_buf;
RGBConverter rgb_conv;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(RGB_R_PIN, OUTPUT);
  pinMode(RGB_G_PIN, OUTPUT);
  pinMode(RGB_B_PIN, OUTPUT);

  pinMode(WHT1_PIN, OUTPUT);
  pinMode(WHT2_PIN, OUTPUT);
  pinMode(BTN_LED_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  int pot_val = smooth_buf.update_buf(analogRead(POT_PIN));

  // set color of RBG LEDs based on pot position
  float hue = constrain(pot_val - POT_MIN, 0, POT_MAX-POT_MIN) / (float)(POT_MAX-POT_MIN);
  byte rgb[3];
  rgb_conv.hslToRgb(hue, 1, 0.5, rgb);
  setColor(rgb[0], rgb[1], rgb[2]);

  // set brightness of white LEDs based on pot position
  int brightness = (int)(hue * 255);
  analogWrite(WHT1_PIN, brightness);
  analogWrite(WHT2_PIN, brightness);
  analogWrite(BTN_LED_PIN, brightness);

  int light_level = analogRead(LIGHTSENSE_PIN);

  Serial.print("lightLevel=");
  Serial.print(light_level);
  Serial.print(" potVal=");
  Serial.print(pot_val);
  Serial.print(" hue=");
  Serial.println(hue);

  delay(5);
}

void setColor(int red, int green, int blue)
{
  analogWrite(RGB_R_PIN, red);
  analogWrite(RGB_G_PIN, green);
  analogWrite(RGB_B_PIN, blue);  
}
