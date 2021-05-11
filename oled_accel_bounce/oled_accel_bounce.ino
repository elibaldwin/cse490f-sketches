/*  Elias Baldwin
 *   
 *  This program creates a bouncing ball on the Adafruit SSD1306 display,
 *  which accelerates around the display using tilt/motion from the accelerometer
 *  and bounces off the walls. Includes an FPS display in the top left corner.
 * 
 *  Uses Adafruit LIS3DH Accelerometer, and works with the Adafruit ESP32 Feather board.
 *  (might work with Arduino Leonardo with the right pin configuration, but likely
 *  would result in lower framerates or out-of-memory issues)
 */

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

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

// Physics constants
const float ELASTICITY = 0.99f; // ball bounce elasticity (applied on bounce)
const float FRICTION = 0.98f;   // ball movement friction (applied every frame)

const float GRAV_SCALER = 0.25f; // scale factor for gravity strength

// Ball position, size and movement variables
float ball_x = SCREEN_WIDTH / 2;
float ball_y = (SCREEN_HEIGHT - 10) / 2;
int ball_rad = 4;

float ball_dx = 0;
float ball_dy = 0;

// FPS tracking
float fps = 0;
unsigned long frame_count = 0;
unsigned long fps_millis = 0;

// Frame timing
int target_frametime = 32; // minimum number of milliseconds between frames
unsigned long frametime_millis = 0;

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  _display.clearDisplay();    
  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK); 

  Serial.println("LIS3DH test!");
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
  Serial.println("LIS3DH found!");
}

void loop() {

  // use frame timing to avoid delays and keep framerate as consistent as possible
  if (millis() > frametime_millis + target_frametime) {
    frametime_millis = millis();
    
    calcFPS();
    
    // On each loop, we'll want to clear the display so we're not writing over
    // previously drawn data
    _display.clearDisplay(); 
  
    physicsUpdate();
  
    _display.drawCircle(round(ball_x), round(ball_y), ball_rad, WHITE);
    drawFPS();
    
    // Render the graphics buffer to screen
    _display.display();
  }
   
}


void physicsUpdate() {
  int pot_val = analogRead(POT_PIN);
  float acc_scale = (float)pot_val / (float)ANALOG_MAX;

  // fetch values from the accelerometer
  sensors_event_t event;
  lis.getEvent(&event);

  // depending on orientation of accelerometer relative to display, 
  // x / y acceleration values might need to be switched or negated
  ball_dx += acc_scale * GRAV_SCALER * -event.acceleration.x;
  ball_dy += acc_scale * GRAV_SCALER * event.acceleration.y;

  // friction decay
  ball_dx *= FRICTION;
  ball_dy *= FRICTION;

  // move ball position using velocity
  ball_x += ball_dx;
  ball_y += ball_dy;

  // collision checks for each 'wall' of the display
  if (ball_x - ball_rad < 0) {
    ball_x = ball_rad;
    ball_dx *= -ELASTICITY;
  }

  if (ball_x + ball_rad + 2 > SCREEN_WIDTH) {
    ball_x = SCREEN_WIDTH - ball_rad - 2;
    ball_dx *= -ELASTICITY;
  }

  if (ball_y - ball_rad < 0) {
    ball_y = ball_rad;
    ball_dy *= -ELASTICITY;
  }

  if (ball_y + ball_rad + 2 > SCREEN_HEIGHT) {
    ball_y = SCREEN_HEIGHT - ball_rad - 2;
    ball_dy *= -ELASTICITY;
  }
}

void calcFPS() {
  unsigned long frame_time = millis() - fps_millis;
  frame_count++;
  if (frame_time > 1000) {
    fps = frame_count / (frame_time / 1000.0);
    fps_millis = millis();
    frame_count = 0;
  }
}

void drawFPS() {
  _display.setCursor(0, 0);
  _display.print(fps);
  _display.print(" fps");
}
