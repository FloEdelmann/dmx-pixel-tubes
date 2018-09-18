#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>
#include <FastLED.h>

// Wifi settings
#define SSID "ssid"
#define PASSPHRASE "passphrase"

// FastLED settings
#define NUM_LEDS 30
#define DATA_PIN 5

CRGB leds[NUM_LEDS];

// Art-Net settings
// some lighting programs send out first Art-Net universe as 0, most as 1
#define ART_NET_UNIVERSE 0
#define MAX_CHANNELS NUM_LEDS * 3

ArtnetWifi artnet;



// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void) {
  WiFi.begin(SSID, PASSPHRASE);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");

  boolean state = true;
  int i = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

    if (i > 20) {
      state = false;
      break;
    }
    i++;
  }

  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return state;
}


void initTest() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(255, 0, 0);
  }
  FastLED.show();
  delay(500);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(0, 255, 0);
  }
  FastLED.show();
  delay(500);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(0, 0, 255);
  }
  FastLED.show();
  delay(500);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(0, 0, 0);
  }
  FastLED.show();
}


void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  Serial.printf("Frame received: Universe %d, length %d, sequence %d\n", universe, length, sequence);

  if (universe != ART_NET_UNIVERSE) {
    return;
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length; i += 3) {
    int led = i / 3;
    if (led < NUM_LEDS) {
      leds[led].setRGB(data[i], data[i + 1], data[i + 2]);
    }
  }

  FastLED.show();
}


void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  initTest();

  ConnectWifi();
  artnet.begin();

  artnet.setArtDmxCallback(onDmxFrame);
}


void loop() {
  artnet.read();
}
