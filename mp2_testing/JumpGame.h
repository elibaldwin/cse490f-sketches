
/*  Elias Baldwin
 *  CSE490 Spring 2021
 *  MP2: Microgame
 *  
 *  This is where the majority of the code I wrote for this project lives.
 *  
 *  This is a simple game modeled vaguely off of Doodle Jump, written to
 *  be used with the Adafruit HUZZAH ESP32. It features hiscore tracking
 *  using Preferences to write to EEPROM, a simple one-button plus
 *  accelerometer control scheme, and gradually increasing difficulty.
 *  
 *  With a vibromotor and piezo speaker attached (and with their pins configured),
 *  it also provides basic haptic and audio feedback while playing the game.
 * 
 */


#include "Buffer.h"
#include "Arduino.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#include <PushButton.h>

#include <ESP32Servo.h>
#include <analogWrite.h>
//#include <tone.h>
#include <ESP32Tone.h>
#include <ESP32PWM.h>

#include <Preferences.h>

const int BUZZ_PIN = 12;
const int BUZZ_DURATION = 150; 

const int PIEZO_PIN = 21;
const int NOISE_DURATION = 900;

#define MAX_PLATFORMS 20
#define TARGET_FPS 30

const float GRAV = -200.0f;        // Gravity scalar value
const float JUMP_IMPULSE = 140.0f; // Jump impulse scalar value
const float ACCEL_SENS = 60.0f;    // Accelerometer sensitivity (side-to-side movement)
const float DX_MAX = 120.0f;

const int PLAYER_WIDTH = 10;
const int PLAYER_HEIGHT = 14;

const int PLATFORM_HEIGHT = 4;
const int PLATFORM_RAD = 2;

enum p_type : uint8_t {
  basic, breakable
};

struct Platform {
  int32_t y;    // y position of the platform in 'world' space
  float x;       // x position of the platform in 'screen' space
  float dx;      // x velocity of movement (only used if 'moving' flag is nonzero)
  uint8_t width; // width of platform
  p_type type;   // type of platform
  uint8_t live;  // 1 if platform should be drawn / used for physics
  uint8_t moving;// 1 if this platform can move
};

enum gamestate {
  menu, play, stasis, dead, scores
};


class JumpGame {
  private:
    Adafruit_SSD1306 *screen;
    Adafruit_LIS3DH *accelerometer;
    PushButton *button;
    Preferences prefs;

    int32_t hiscores[10];
  
    Buffer<Platform, MAX_PLATFORMS> platforms;
    int32_t last_plat_height;
    int32_t view_height;
    gamestate state;

    double player_y; 
    float player_x;

    float player_dx;
    float player_dy;

    uint32_t last_frame_micros = 0;
    uint32_t target_frametime = 1000000 / TARGET_FPS;

    uint32_t buzz_ms;
    uint32_t noise_ms;

    int n_lives = 3;
    bool first_jump = true;

    bool button_click = false;

  public:
    JumpGame(Adafruit_SSD1306 *disp, Adafruit_LIS3DH *accel, PushButton *btn) {
      screen = disp;
      accelerometer = accel;
      button = btn;
    }

    void reset() {
      view_height = -200;
      state = menu;
      player_y = -136;
      player_x = 27;
      first_jump = true;
      last_plat_height = 0;
      n_lives = 3;

      platforms.clear();
      Platform p1 = {-170, 22.0f, 0.0f, 20, basic, 1, 0};
      platforms.push_back(p1);

      prefs.begin("jump", false);
      size_t scores_size = prefs.getBytesLength("hiscores");
      prefs.getBytes("hiscores", (void*)hiscores, scores_size);
      prefs.end();

      Serial.println("finished game reset");
    }

    void time_reset() {
      last_frame_micros = micros();
    }

    void update() {
      uint32_t ts = micros();
      
      button->update();
      if (button->isClicked()) button_click = true;
      
      if (last_frame_micros == 0 || ts > last_frame_micros + target_frametime) {
        uint32_t frametime = ts - last_frame_micros;
        last_frame_micros = ts;

        if (state == menu) {
          menu_update();
        } else if (state == play) {
          play_update(frametime);
        } else if (state == stasis) {
          pause_update();
        } else if (state == dead) {
          dead_update();
        } else if (state == scores) {
          scores_update();
        }
        
        button_click = false;
      }
    }

    void menu_update() {
      
      screen->clearDisplay();
      draw_game(false);

      screen->setTextSize(2);
      screen->setCursor(3, 20);
      screen->print("JUMP!");

      screen->display();
      
      if (button_click) {
        state = play;
        start(false);
      }
    }

    void pause_update() {
      screen->clearDisplay();
      draw_game(true);

      screen->setTextSize(2);
      screen->setCursor(3, 25);
      screen->print("PAUSE");

      screen->display();

      if (button_click) {
        state = play;
        start(true);
      }
    }

    void start(bool from_pause) {
      for (int i = 3; i > 0; i--) {
        screen->clearDisplay();
        draw_game(from_pause);

        screen->setTextSize(2);
        screen->setCursor(27, 30);
        screen->print(i);
        screen->display();
        
        delay(1000);
      }

      time_reset();
    }

    void play_update(uint32_t frametime) {
      screen->clearDisplay();
      
      gen_platforms();
      
      player_update(frametime);
      if (millis() > buzz_ms) {
        noTone(BUZZ_PIN);
      }

      if (millis() < noise_ms) {
        int diff = (noise_ms - millis()) / 150;
        if (diff % 2 == 1) {
          tone(PIEZO_PIN, 100);
        } else {
          noTone(PIEZO_PIN);
        }
      } 

      draw_game(view_height >= 0);
        
      screen->display();

      if (button_click) {
        state = stasis;
      }
    }

    void dead_update() {
      screen->clearDisplay();
      draw_end_score();
      screen->display();

      if (button_click) {
        log_score();
        state = scores;
      }
    }

    void game_over() {
      screen->clearDisplay();
      draw_end_score();
      screen->display();
    }

    void draw_end_score() {
      screen->setTextSize(2);
      screen->setCursor(8, 10);
      screen->print("GAME");
      screen->setCursor(8, 36);
      screen->print("OVER");
      
      if (view_height >= 10000) {
        screen->setCursor(3, 80);
      } else if (view_height >= 1000) {
        screen->setCursor(8, 80);
      } else {
        screen->setCursor(13, 80);
      }
      screen->print(view_height);

      screen->setTextSize(1);
      screen->setCursor(15, 65);
      screen->print("Score:");

      screen->setTextSize(3);
      screen->setCursor(24, 100);
      screen->print("\1");
    }

    void log_score() {
      int pos = 0;
      while (pos < 10) {
        if (view_height > hiscores[pos]) break;
        pos++;
      }

      if (pos == 10) return;

      for (int i = 9; i > pos; i--) {
        hiscores[i] = hiscores[i-1];
      }
      hiscores[pos] = view_height;

      prefs.begin("jump", false);
      prefs.putBytes("hiscores", (void*)hiscores, 40);
      prefs.end();
    }

    void scores_update() {
      screen->clearDisplay();

      screen->setTextSize(1);
      screen->setCursor(5, 5);
      screen->print("HISCORES:");

      for (int i = 0; i < 10; i++) {
        if (hiscores[i] == view_height) {
          screen->setTextColor(BLACK, WHITE);
        } else {
          screen->setTextColor(WHITE, BLACK);
        }
        screen->setCursor(5, 17 + 10 * i);
        screen->print(i+1);
        screen->print(".  ");
        if (i < 9) screen->print(" ");
        screen->print(hiscores[i]);
      }

      screen->setTextColor(WHITE, BLACK);

      screen->display();
      if (button_click) {
        reset();
        state = menu;
      }
    }

    void draw_game(bool draw_header) {
      draw_player();
      draw_platforms(); 
      if (draw_header) {
        screen->setCursor(0,0);
        screen->setTextSize(1);
        for (int i = 0; i < 4; i++) {
          if (i < n_lives) screen->print("\3");
          else             screen->print(" ");
        }
        screen->print(view_height);
      }  
    }

    void draw_platforms() {
      Platform curr;
      // discard platforms that have moved out of view
      while(platforms[0].y < view_height) platforms.pop_front();
      
      for (int i = 0; i < platforms.size(); i++) {
        curr = platforms[i];
        if (!curr.live) continue; // skip dead platforms

        // y position of platform in screen space
        int screen_y = 128 - (curr.y - view_height);
        
        if (screen_y < -4) break; // stop if we've reached platforms that aren't yet onscreen

        if (curr.type == breakable) {
          screen->drawRoundRect(round(curr.x), screen_y, curr.width, PLATFORM_HEIGHT, PLATFORM_RAD, WHITE);
        } else if (curr.type == basic) {
          screen->fillRoundRect(round(curr.x), screen_y, curr.width, PLATFORM_HEIGHT, PLATFORM_RAD, WHITE);
        }

        if (curr.moving && state == play) {
          curr.x += curr.dx;
          if (curr.x < 0 && curr.dx < 0) {
            curr.x = 0;
            curr.dx = -curr.dx;
          } else if (curr.x > 49 && curr.dx > 0) {
            curr.x = 49;
            curr.dx = -curr.dx;
          }
          platforms[i] = curr;
        }
      }
    }

    void gen_platforms() {
      int difficulty = view_height / 500 + 1;
      while (platforms.size() < MAX_PLATFORMS) {
        int ypos = last_plat_height + random(8, 50);
        int xpos = random(0, 50);
        int type = random(0, difficulty + 1);
        int can_move = random(0, min(difficulty, 4));
        float xvel = random(1, min(difficulty, 5) + 1) / 5.0;
        Platform p = {ypos, xpos, xvel, 15, (p_type)constrain(type, 0, 1), 1, can_move};
        platforms.push_back(p);
        last_plat_height = ypos;
      }
    }

    void draw_player() {
      char c;
      if (player_dy < 0) {
        c = (char)1;
      } else {
        c = (char)2;
      }
      screen->drawChar(round(player_x), 128 - round(player_y - view_height), c, WHITE, BLACK, 2);
      if (round(player_x) > 54) {
        screen->drawChar(round(player_x) - 64, 128 - round(player_y - view_height), c, WHITE, BLACK, 2);
      }
    }

    void player_update(uint32_t frametime) {
      float t_delta = (float)frametime / 1000000.0f;

      sensors_event_t event;
      accelerometer->getEvent(&event);

      player_dy += t_delta * GRAV;
      player_dx += t_delta * -event.acceleration.y * ACCEL_SENS;
      player_dx = (player_dx > DX_MAX) ? DX_MAX : ((player_dx < -DX_MAX) ? -DX_MAX : player_dx); 

      player_y += t_delta * player_dy;
      player_x += t_delta * player_dx;

      if (player_x > 64) {
        player_x -= 64;
      } else if (player_x < 0) {
        player_x += 64;
      }

      int32_t pview_height = round(player_y - 64);
      view_height = max(view_height, pview_height);


      // player has fallen off the screen
      if (player_y - PLAYER_HEIGHT < view_height) {
        // TODO: do something other than bounce (enter dead state)
        if (n_lives == 0) {
          state = dead;
          game_over();
          return;
        }
        
        player_y = view_height + PLAYER_HEIGHT;
        player_dy = JUMP_IMPULSE * 2;
        n_lives -= 1;
        noise_ms = millis() + NOISE_DURATION;
      }

      // platform collisions
      // (only consider if player is moving downward)
      if (player_dy <= 0) {
        Platform curr;
        for (int i = 0; i < platforms.size(); i++) {
          curr = platforms[i];

          if (!curr.live) continue;

          // short-circuit break if this platform is above the player
          // (all the rest of the platforms will be above the player as well)
          if (player_y - PLAYER_HEIGHT < curr.y - PLATFORM_HEIGHT) break;
  
          if ( (player_y - PLAYER_HEIGHT < curr.y) &&
               ((player_x + PLAYER_WIDTH > curr.x) && (player_x < curr.x + curr.width) ||
                (player_x + PLAYER_WIDTH - 64 > curr.x) && (player_x - 64 < curr.x + curr.width)) ) {
            player_y = curr.y + PLAYER_HEIGHT;
            
            if (first_jump) {
              player_dy = JUMP_IMPULSE * 3;
              buzz_ms = millis() + BUZZ_DURATION * 3;
              first_jump = false;
            } else {
              player_dy = JUMP_IMPULSE;
              buzz_ms = millis() + BUZZ_DURATION;
            }  

            if (curr.type == breakable) {
              platforms[i].live = 0;
              tone(BUZZ_PIN, 5000);
            } else {
              tone(BUZZ_PIN, 500);
            }
          }
        }
      }
    }

};
