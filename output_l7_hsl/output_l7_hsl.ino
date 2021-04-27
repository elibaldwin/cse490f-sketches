/*
 * This example cross fades the hue, which is then converted to RGB. 
 * This implemention is much cleaner and more understandable than CrossFadeRGB: 
 * https://github.com/makeabilitylab/arduino/tree/master/Basics/analogWrite/CrossFadeRGB
 * 
 * Uses an RGB to HSL and HSL to RGB conversion library from here:
 *   https://github.com/ratkins/RGBConverter
 *   
 * By Jon E. Froehlich
 * @jonfroehlich
 * http://makeabilitylab.io
 * 
 * For a walkthrough and circuit diagram, see:
 * https://makeabilitylab.github.io/physcomp/arduino/rgb-led-fade
 * 
 */

#include "src/RGBConverter/RGBConverter.h"

const int BUF_SIZE = 16;
class RingBuffer {
  private:
    int buf[BUF_SIZE];
    int sum;
    int i;

  public:
    int update_buf(int val) {
      sum -= buf[i];
      sum += val;
      buf[i] = val;
      i = (i+1) % BUF_SIZE;
      return sum / BUF_SIZE;
    }
};

// Change this to based on whether you are using a common anode or common cathode
// RGB LED. See: https://makeabilitylab.github.io/physcomp/arduino/rgb-led 
// If you are working with a common cathode RGB LED, set this to false.
const boolean COMMON_ANODE = false; 

const int RGB_RED_PIN = 6;
const int RGB_GREEN_PIN  = 5;
const int RGB_BLUE_PIN  = 3;
const int DELAY_INTERVAL = 5; // interval in ms between incrementing hues
const byte MAX_RGB_VALUE = 255;

const int POT_PIN = A0;
RingBuffer smooth_buf;
const int POT_MIN = 125;
const int POT_MAX = 930;

float _hue = 0; //hue varies between 0 - 1
float _step = 0.001f;

RGBConverter _rgbConverter;

void setup() {
  // Set the RGB pins to output
  pinMode(RGB_RED_PIN, OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN, OUTPUT);

  // Turn on Serial so we can verify expected colors via Serial Monitor
  Serial.begin(9600);   
}

void loop() {
  int potVal = smooth_buf.update_buf(analogRead(POT_PIN));
  _hue =  constrain(potVal - POT_MIN, 0, POT_MAX-POT_MIN) / (float)(POT_MAX-POT_MIN);

  // Convert current hue, saturation, and lightness to RGB
  // The library assumes hue, saturation, and lightness range from 0 - 1
  // and that RGB ranges from 0 - 255
  // If lightness is equal to 1, then the RGB LED will be white
  byte rgb[3];
  _rgbConverter.hslToRgb(_hue, 1, 0.5, rgb);

  Serial.print("potVal=");
  Serial.print(potVal);
  Serial.print(" hue=");
  Serial.print(_hue);
  Serial.print(" r=");
  Serial.print(rgb[0]);
  Serial.print(" g=");
  Serial.print(rgb[1]);
  Serial.print(" b=");
  Serial.println(rgb[2]);
  
  setColor(rgb[0], rgb[1], rgb[2]); 

  
  
  delay(DELAY_INTERVAL);
}

/**
 * setColor takes in values between 0 - 255 for the amount of red, green, and blue, respectively
 * where 255 is the maximum amount of that color and 0 is none of that color. You can illuminate
 * all colors by intermixing different combinations of red, green, and blue. 
 * 
 * This function is based on https://gist.github.com/jamesotron/766994
 * 
 */
void setColor(int red, int green, int blue)
{
  // If a common anode LED, invert values
  if(COMMON_ANODE == true){
    red = MAX_RGB_VALUE - red;
    green = MAX_RGB_VALUE - green;
    blue = MAX_RGB_VALUE - blue;
  }
  analogWrite(RGB_RED_PIN, red);
  analogWrite(RGB_GREEN_PIN, green);
  analogWrite(RGB_BLUE_PIN, blue);  
}
