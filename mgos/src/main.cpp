#include "mgos.h"
#include "mgos_gpio.h"
#include "mgos_wifi.h"
#include "mgos_rpc.h"
#include "Adafruit_Neopixel.h"
#include "mgos_iterator.h"

#include "Arduino.h"

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

void rainbow3(void *arg, int round) {
  Adafruit_NeoPixel *strip = static_cast<Adafruit_NeoPixel*>(arg);
  int i;
  int direction = (round / strip->numPixels()) & 1;
  LOG(LL_INFO, ("Hello: %d direction %d", round, direction));
  for(i=0; i<=strip->numPixels(); i++) {
    if (direction == 1) {
      strip->setPixelColor(i, Wheel((byte)(8 * (i + round))));
    } else {
      strip->setPixelColor(i, Wheel((byte)((8 * (i - round)))));
    }
  }
  strip->show();
  
  (void) round;
}

static int rpcSetPixelColorOne(struct mg_rpc_request_info *ri, Adafruit_NeoPixel *strip, int led, int red, int green, int blue, int white) {
  if (led == -1) {
    mg_rpc_send_errorf(ri, 400, "led value is required.");
    return -1;
  }
  if (red < 0 || red > 255) {
    mg_rpc_send_errorf(ri, 400, "invalid red value: %d", red);
    return -1;
  }
  if (green < 0 || green > 255) {
    mg_rpc_send_errorf(ri, 400, "invalid green value: %d", green);
    return -1;
  }
  if (blue < 0 || blue > 255) {
    mg_rpc_send_errorf(ri, 400, "invalid blue value: %d", blue);
    return -1;
  }
  if (white < 0 || white > 255) {
    mg_rpc_send_errorf(ri, 400, "invalid white value: %d", white);
    return -1;
  }

  LOG(LL_INFO, ("%d/%d = #%02x%02x%02x/%02x", led, strip->numPixels(), red, green, blue, white));
  LOG(LL_INFO, ("strip: %p", strip));
  strip->setPixelColor(led, red, green, blue, white);

  return 0;
}

static void rpcSetPixelColor(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args) {
  int led = -1, red = 0, green = 0, blue = 0, white = 0;
  LOG(LL_INFO, ("%s", ri->args_fmt));
  LOG(LL_INFO, ("%.*s", args.len, args.p));
  json_scanf(args.p, args.len, ri->args_fmt, &led, &red, &green, &blue, &white);

  Adafruit_NeoPixel *strip = static_cast<Adafruit_NeoPixel*>(cb_arg);
  rpcSetPixelColorOne(ri, strip, led, red, green, blue, white);
  strip->show();

  (void) fi;
}

static void rpcSetPixelColorMany(struct mg_rpc_request_info *ri, const char *args, const char *src, void *user_data) {
  Adafruit_NeoPixel *strip = static_cast<Adafruit_NeoPixel*>(user_data);

  int i = 0, len = strlen(args);
  int led = -1, red = 0, green = 0, blue = 0, white = 0;
  struct json_token t;
  for (i = 0; json_scanf_array_elem(args, len, ".pixels", i, &t) > 0; i++) {
    json_scanf(t.ptr, t.len, "{ led: %d, red: %d, green: %d, blue: %d, white: %d }", &led, &red, &green, &blue, &white);
    if (rpcSetPixelColorOne(ri, strip, led, red, green, blue, white) == -1) {
      return; // error
    }
  }
  strip->show();

  mg_rpc_send_responsef(ri, "OK");

  (void) src; // only used with mqtt
}

enum mgos_app_init_result mgos_app_init(void) {
  const int count = 33;
  Adafruit_NeoPixel *strip = new Adafruit_NeoPixel(count, 5, NEO_GRBW);
  strip->begin();

  rainbow3(strip, 0);

  struct mg_rpc *c = mgos_rpc_get_global();
  LOG(LL_INFO, ("strip: %p", strip));
  mg_rpc_add_handler(c, "NeoPixel.SetPixelColor", "{ led: %d, red: %d, green: %d, blue: %d, white %d }", rpcSetPixelColor, strip);
  //mg_rpc_add_handler(c, "NeoPixel.SetPixelColorMany", "{ pixels: %M }", rpcSetPixelColorMany, strip);
  mgos_rpc_add_handler("NeoPixel.SetPixelColorMany", rpcSetPixelColorMany, strip);
  
  return MGOS_APP_INIT_SUCCESS;
}