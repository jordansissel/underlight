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

void Wheel(byte pos, byte &red, byte &green, byte &blue) {
  pos = 255 - pos;
  if(pos < 85) {
    red = 255 - pos * 3;
    green = 0;
    blue = pos * 3;
  } else if (pos < 170) {
    pos -= 85;
    red = 0;
    green = pos * 3;
    blue = 255 - pos * 3;
  } else {
    pos -= 170;
    red = pos * 3;
    green = 255 - pos * 3;
    blue = 0;
  }
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

// static unsigned char hex(const char p) {
//   if (isdigit(p))
//     return p - '0';
//   if (isupper(p) && p <= 'F')
//     return p - 'A';
//   if (islower(p) && p <= 'f')
//     return p - 'a';
//   return -1;
// }

// static unsigned char hex2c(const char *p) {
//   return hex(p[0]) << 4 + hex(p[1]);
// }

struct animation {
  Adafruit_NeoPixel *strip;
  const char *animation;
  int position = 0;
  int len;
};

enum AnimationCode : byte {
  RGBW = 0, // 5 bytes led + each color + white
  RGB = 1, // 3 bytes led + each color
  Red = 2, // 1 byte led + red
  Green = 3, // 1 byte led + green
  Blue = 4, // 1 byte led + blue
  Rainbow = 5, // 1 byte led + rainbow
  Reserved = 6, // Excitement awaits.
  EndOfPixels = 7,
};


void handle_opcode(AnimationCode opcode, struct animation *sequence, byte &red, byte &green, byte &blue, byte &white) {
  white = red = green = blue = 0;
  switch (opcode) {
  case RGBW:
    red = sequence->animation[0 + sequence->position];
    green = sequence->animation[1 + sequence->position];
    blue = sequence->animation[2 + sequence->position];
    white = sequence->animation[3 + sequence->position];
    sequence->position += 4;
    break;
  case RGB:
    red = sequence->animation[0 + sequence->position];
    green = sequence->animation[1 + sequence->position];
    blue = sequence->animation[2 + sequence->position];
    sequence->position += 3;
    break;
  case Red:
    red = sequence->animation[0 + sequence->position];
    sequence->position += 1;
    break;
  case Green:
    green = sequence->animation[0 + sequence->position];
    sequence->position += 1;
    break;
  case Blue:
    blue = sequence->animation[0 + sequence->position];
    sequence->position += 1;
    break;
  case Rainbow:
    Wheel(sequence->animation[0 + sequence->position], red, green, blue);
    sequence->position += 1;
    break;
  case Reserved:
    LOG(LL_INFO, ("SHOULD NOT GET HERE - RESERVED"));
    break;
  case EndOfPixels:
    LOG(LL_INFO, ("SHOULD NOT GET HERE - END OF PIXELS"));
    break;
  }
}

void animate(void *param) {
  struct animation *sequence = static_cast<struct animation *>(param);
  int i = 0;

  // read up to the first time we get 0xFFFFFFFFFF
  while (sequence->animation != NULL) {
    byte header = sequence->animation[0 + sequence->position];

    auto opcode = static_cast<AnimationCode>(header >> 5); // upper 3 bits for opcode

    sequence->position++;

    if (opcode == EndOfPixels) {
      break;
    }

    byte led = header & 31;  // lower 5 bits for led
    //LOG(LL_INFO, ("Byte header: %02x - %d - %d", header, led, opcode));

    byte red, green, blue, white;
    handle_opcode(opcode, sequence, red, green, blue, white);
    sequence->strip->setPixelColor(led, red, green, blue, white);
    i++;
  }
  //LOG(LL_INFO, ("Rendering %d pixels", i));

  sequence->strip->show();
  //LOG(LL_INFO, ("Rendering %d pixels done", i));
}

bool want_animate(void *param) {
  struct animation *sequence = static_cast<struct animation *>(param);
  bool want = sequence->position < sequence->len;
  if (!want) {
    free((void *) sequence->animation);
    delete(sequence);
  }
  //LOG(LL_INFO, ("want animate: %s", want ? "true" : "false"));
  return want;
}

mgos_iterator_id mgos_iterator(int msecs, predicate has_next, timer_callback cb, void *arg);


//static void rpcAnimate(struct mg_rpc_request_info *ri, const char *args, const char *src, void *user_data) {
static void rpcAnimate(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args) {
  // hexadecimal encoded: led, red, green, blue, white.
  // sequence repeated.
  struct animation *sequence = new struct animation();
  sequence->strip = static_cast<Adafruit_NeoPixel *>(cb_arg);
  sequence->animation = (char *) malloc(args.len);
  sequence->len = args.len;
  memcpy((char *)sequence->animation, (char *)args.p, args.len);
  // XXX: must free sequence->animation + sequence

  mgos_iterator(16, want_animate, animate, sequence);

  mg_rpc_send_responsef(ri, "OK");

  (void) fi;
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
  mg_rpc_add_handler(c, "NeoPixel.Animate", "{}", rpcAnimate, strip);
  
  return MGOS_APP_INIT_SUCCESS;
}