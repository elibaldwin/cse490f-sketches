#include <Arduino.h>

class Blinker{
  private:
    const int _pin;                 // output pin
    const unsigned long _interval;  // blink interval in ms

    int _state;                     // current state (either HIGH OR LOW)
    unsigned long _lastToggledTimestamp; // last state toggle in ms

  public:
    Blinker(int pin, unsigned long blinkInterval);
    void update();
};
