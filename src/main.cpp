#include "mgos.h"
#include "mgos_gpio.h"
#include "mgos_wifi.h"
#include "Adafruit_Neopixel.h"
#include "iterator.h"

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return Color(255 - WheelPos * 3, 0, WheelPos * 3,0);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return Color(0, WheelPos * 3, 255 - WheelPos * 3,0);
  }
  WheelPos -= 170;
  return Color(WheelPos * 3, 255 - WheelPos * 3, 0,0);
}

void rainbow(void *arg) {
  Adafruit_NeoPixel *strip = static_cast<Adafruit_NeoPixel*>(arg);
  int i;
  LOG(LL_INFO, ("Hello: %d", strip->numPixels()));
  //strip->setBrightness(255);
  for(i=0; i<=strip->numPixels(); i++) {
    //strip->setPixelColor(i, Wheel(((i + round) * 8) & 255));
    strip->setPixelColor(i, Wheel(i * 8));
  }
  strip->show();

  (void) round;
}

void rainbow2(void *arg) {
  Adafruit_NeoPixel *strip = static_cast<Adafruit_NeoPixel*>(arg);
  int i;
  LOG(LL_INFO, ("Hello: %d", strip->numPixels()));
  for(i=0; i<=strip->numPixels(); i++) {
    strip->setPixelColor(i, Wheel((i + 100) * 8));
  }
  strip->show();

  (void) round;
}

void rainbow3(void *arg, int round) {
  Adafruit_NeoPixel *strip = static_cast<Adafruit_NeoPixel*>(arg);
  int i;
  int direction = (round / strip->numPixels()) & 1;
  LOG(LL_INFO, ("Hello: %d direction %d", round, direction));
  for(i=0; i<=strip->numPixels(); i++) {
    if (direction == 1) {
      strip->setPixelColor(i, (i + round) * 8, 0, 0, 0);
    } else {
      strip->setPixelColor(strip->numPixels() - i, (i + round) * 8, 0, 0, 0);
    }
  }
  strip->show();

  (void) round;
}

void greeting(void *arg, int i) {
  LOG(LL_INFO, ("greet: %p %d", arg, i));
  (void) arg;
}

enum mgos_app_init_result mgos_app_init(void) {
  const int count = 33;
  Adafruit_NeoPixel *strip = new Adafruit_NeoPixel(count, 5, NEO_GRBW);
  strip->begin();
  strip->clear();
  strip->show();

  mgos_set_timer(100, false, rainbow, strip);
  mgos_set_timer(1100, false, rainbow2, strip);
  mgos_iterator_count(16, 10, greeting, strip);
  mgos_iterator_count(16, 1000, rainbow3, strip);

  return MGOS_APP_INIT_SUCCESS;
}