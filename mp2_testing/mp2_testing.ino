/*  Elias Baldwin
 *  CSE490 Spring 2021
 *  MP2: Microgame
 *  
 *  Main sketch file:
 *  
 *  This handles initialization and pin configuration for the Adafruit
 *  OLED and Accelerometer. For the code that contains all of the actual
 *  game logic, see JumpGame.h.
 */


#include <PushButton.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#include "JumpGame.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Used for software SPI - these should be GPIO pins on the ESP32
#define LIS3DH_CLK 18
#define LIS3DH_MISO 32
#define LIS3DH_MOSI 19
#define LIS3DH_CS 14

// Accelerometer object
Adafruit_LIS3DH lis(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);

// Potentiometer used to control strength of gravity acceleration
const int POT_PIN = A0;
const int ANALOG_MAX = 4095;

const int BUTTON_PIN = 17;
PushButton pause_button(BUTTON_PIN);

JumpGame game(&_display, &lis, &pause_button);
bool paused = true;

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  //pause_button.setActiveLogic(LOW);
  
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  _display.clearDisplay();    
  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK); 

  // put the screen in a vertical orientation
  _display.setRotation(3);

  Serial.println("LIS3DH test!");
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
  Serial.println("LIS3DH found!");

  randomSeed(analogRead(A0));

  game.reset();
}

void loop() {
  game.update();
}
