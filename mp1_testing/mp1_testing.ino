#include <PushButton.h>
#include "RingBuffer.h"
#include "src/RGBConverter/RGBConverter.h"

const int POT_PIN = A0;

// configuration values for calibrating potentiometer range
const int POT_MIN = 140; 
const int POT_MAX = 960;

const int LIGHTSENSE_PIN = A1;

// configuration values for light sensor range
const int LIGHT_MIN = 0;
const int LIGHT_MAX = 250;

// pins for the two parallel RGB LEDs
const int RGB_R_PIN = 6;
const int RGB_G_PIN = 5;
const int RGB_B_PIN = 3;

// pins for each pair of parallel white LEDs
const int WHT1_PIN = 9;
const int WHT2_PIN = 10;

const int WHT_BRIGHTNESS = 180;

// pin for the red button LED
const int BTN_LED_PIN = 11;
const int BTN_PIN = 12;

PushButton mode_btn(BTN_PIN);

RingBuffer<16> pot_buf;
RingBuffer<64> light_buf;
RGBConverter rgb_conv;

int mode = 0;
const int N_MODES = 4;

long last_update_millis = 0;
const int DELAY = 20;

const int FADE_INTERVAL = 5000;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(RGB_R_PIN, OUTPUT);
  pinMode(RGB_G_PIN, OUTPUT);
  pinMode(RGB_B_PIN, OUTPUT);

  pinMode(WHT1_PIN, OUTPUT);
  pinMode(WHT2_PIN, OUTPUT);
  pinMode(BTN_LED_PIN, OUTPUT);

  pinMode(BTN_PIN, INPUT_PULLUP);
  mode_btn.setActiveLogic(LOW);
}

void loop() {
  mode_btn.update();
  if (mode_btn.isClicked()) {
    mode = (mode+1) % N_MODES;
  }

  if (millis() > last_update_millis + DELAY) {
    last_update_millis = millis();
    // read from analog inputs, updating ring buffers
    int light_level = light_buf.update_buf(analogRead(LIGHTSENSE_PIN));
    int pot_val = pot_buf.update_buf(analogRead(POT_PIN));

    switch(mode) {
      case 0: // MODE 0: OFF
        set_hue(0, 0, true);
        break;
      case 1: // MODE 1: Crossfade w/ inversely proportional brightness
        step_mode_1();
        break;
      case 2: // MODE 2: Rotation-based color selection
        step_mode_2();
        break;
      case 3: // MODE 3: Rotation-based color breathing
        step_mode_3();
        break;
    }
  
    Serial.print("mode=");
    Serial.print(mode);
    Serial.print(" lightLevel=");
    Serial.print(light_level);
    Serial.print(" potVal=");
    Serial.println(pot_val);
  }
  
}

void step_mode_1() {
  float lightness = constrain(light_buf.get() - LIGHT_MIN, 0, LIGHT_MAX-LIGHT_MIN) / (float)(LIGHT_MAX-LIGHT_MIN);
  float darkness = 1-lightness;

  float hue = (millis() % FADE_INTERVAL) / (float)FADE_INTERVAL;

  set_hue(hue, darkness, true);
}

void step_mode_2() {
  float hue = constrain(pot_buf.get() - POT_MIN, 0, POT_MAX-POT_MIN) / (float)(POT_MAX-POT_MIN);
  set_hue(hue, 1.0, true);
}

void step_mode_3() {
  float hue = constrain(pot_buf.get() - POT_MIN, 0, POT_MAX-POT_MIN) / (float)(POT_MAX-POT_MIN);
  float brightness = (sin(double(millis()) / (FADE_INTERVAL / 5.0)) + 1) / 2.0;

  set_hue(hue, brightness, false);
}

// expects hue in    range [0.0, 1.0]
// and brightness in range [0.0, 1.0]
void set_hue(float hue, float brightness, boolean fade_whites)
{
  byte rgb[3];
  rgb_conv.hslToRgb(hue, 1, 0.5, rgb);
  analogWrite(RGB_R_PIN, (int)(rgb[0] * brightness));
  analogWrite(RGB_G_PIN, (int)(rgb[1] * brightness));
  analogWrite(RGB_B_PIN, (int)(rgb[2] * brightness));

  if (fade_whites) {
    // update brightnesses of white LEDs and the button LED
    analogWrite(WHT1_PIN, (int)(rgb[0] * brightness));
    analogWrite(WHT2_PIN, (int)(rgb[1] * brightness));
    analogWrite(BTN_LED_PIN, (int)(rgb[2] * brightness));
  } else {
    analogWrite(WHT1_PIN, (int)(WHT_BRIGHTNESS * brightness));
    analogWrite(WHT2_PIN, (int)(WHT_BRIGHTNESS * brightness));
    analogWrite(BTN_LED_PIN, (int)(WHT_BRIGHTNESS * brightness));
  }
  
}
