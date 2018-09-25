#include <Basecamp.hpp>
#include <ArtnetWifi.h>
#include <FastLED.h>

// Wifi settings
Basecamp iot{
  Basecamp::SetupModeWifiEncryption::secured,
  Basecamp::ConfigurationUI::always
};

// FastLED settings
#define NUM_LEDS 30
#define DATA_PIN 5

CRGB leds[NUM_LEDS];

// Art-Net settings
// some lighting programs send out first Art-Net universe as 0, most as 1
#define ART_NET_UNIVERSE 0
#define MAX_CHANNELS NUM_LEDS * 3

ArtnetWifi artnet;



void initTest() {
  fill_solid(leds, NUM_LEDS, CRGB(255, 0, 0));
  FastLED.show();
  delay(500);

  fill_solid(leds, NUM_LEDS, CRGB(0, 255, 0));
  FastLED.show();
  delay(500);

  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 255));
  FastLED.show();
  delay(500);

  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  FastLED.show();
}


void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  if (universe != ART_NET_UNIVERSE) {
    return;
  }

  // DMX channel 1: Dimmer
  if (length >= 1) {
    FastLED.setBrightness(data[0]);
  }

  // DMX channel 2: Color Temperature
  // DMX channel 3: Color Macros
  // DMX channel 4: Auto Programs
  // DMX channel 5: RGB / HSV selection & reversing pixel order

  // DMX channels 6...125: single pixel
  for (int i = 5; (i + 3) < length; i += 4) {
    int led = (i - 5) / 4;
    if (led < NUM_LEDS) {
      leds[led].setRGB(data[i + 1], data[i + 2], data[i + 3]);
      leds[led].nscale8(data[i]);
    }
  }

  FastLED.show();
}

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  initTest();

  iot.begin("LEDs4TheWin!");
  artnet.begin();

  artnet.setArtDmxCallback(onDmxFrame);
}

void loop() {
  artnet.read();
}
