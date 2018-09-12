#include <FastLED.h>

#define NUM_LEDS 30
#define DATA_PIN 5

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
  for(int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot] = CRGB::Red;
  }
  FastLED.show();
  delay(1000);

  for(int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot] = CRGB::Green;
  }
  FastLED.show();
  delay(1000);

  for(int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot] = CRGB::Blue;
  }
  FastLED.show();
  delay(1000);

  for(int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot] = CRGB::White;
  }
  FastLED.show();
  delay(1000);
}
