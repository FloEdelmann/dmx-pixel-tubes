#include <Basecamp.hpp>
#include <ArtnetWifi.h>
#include <FastLED.h>
#include <WS2812FX.h>

// Wifi settings
Basecamp iot{
  Basecamp::SetupModeWifiEncryption::secured,
  Basecamp::ConfigurationUI::always
};

// FastLED settings
#define NUM_LEDS 30
#define LED_PIN 5

CRGB leds[NUM_LEDS];

// WS2812FX settings
#define WS2812FX_FAKE_LED_PIN 4
#define NUM_EFFECT_COLORS 3

WS2812FX ws2812fx = WS2812FX(NUM_LEDS, WS2812FX_FAKE_LED_PIN, NEO_RGB);
WS2812FX::Segment *effectSegment;
CRGB effectColors[NUM_EFFECT_COLORS];

// ESP32-to-ESP32 communication settings
#define RX_PIN 14
#define TX_PIN 19

uint8_t pixelTubeNumberInLine;

// Art-Net settings
// some lighting programs send out first Art-Net universe as 0, most as 1
#define MAX_CHANNELS NUM_LEDS * 3
uint16_t artNetUniverse;
uint16_t artNetStartAddress;

ArtnetWifi artnet;
unsigned long lastArtNetPacketTime = 0;

typedef enum {
  tooManyPixelTubesInLine,
  unconfigured,
  noWiFi,
  noArtNet,
  receiving
} PixelTubeState;

// state variables
PixelTubeState currentState;
bool isEffectEnabled = false;
bool isReversed = true;
bool isHsv = false;



uint8_t getPixelTubeNumberInLine() {
  int lastReceived = -1;
  uint8_t repeatCount = 1; // how often the same number was received;

  unsigned long start = millis();
  unsigned long now = start;

  while (now - start < 10000) {
    if (Serial1.available()) {
      // previous ESP32 has sent their Pixel Tube number

      int received = Serial1.read();
      Serial.printf("Received %d.\n", received);
      if (lastReceived == received) {
        repeatCount++;

        if (repeatCount == 10) {
          return received + 1;
        }
      }
      else {
        lastReceived = received;
        repeatCount = 1;
      }
    }

    delay(10);
    now = millis();
  }

  // nothing received in 7 seconds, so this seems to be the first in this line
  // NOTE: The first Pixel Tube usually receives multiple zeros from the open
  //       connection, so this is just a fallback
  return 1;
}


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


void changeState(PixelTubeState newState) {
  currentState = newState;

  // reset all LEDs to off
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));
  FastLED.show();
}


void showError(uint8_t red, uint8_t green, uint8_t blue) {
  // switch first LED red on and off roughly every 500ms
  if ((millis() % 1000) < 500) {
    leds[NUM_LEDS - 1].setRGB(255, 0, 0);
  }
  else {
    leds[NUM_LEDS - 1].setRGB(0, 0, 0);
  }

  leds[NUM_LEDS - 2].setRGB(red, green, blue);
  FastLED.show();
}


void ws2812fxShowFunction() {
  if (isEffectEnabled) {
    memcpy(leds, ws2812fx.getPixels(), NUM_LEDS * 3);
  }
}


ColorTemperature getColorTemperatureFromDmxValue(uint8_t dmxValue) {
  if (dmxValue <= 29) return UncorrectedTemperature; // 0...29: default
  if (dmxValue <= 54) return Candle; // 30...54: 1900K
  if (dmxValue <= 79) return Tungsten40W; // 55...79: 2600K
  if (dmxValue <= 104) return Tungsten100W; // 80...104: 2850K
  if (dmxValue <= 129) return Halogen; // 105...129: 3200K
  if (dmxValue <= 154) return CarbonArc; // 130...154: 5200K
  if (dmxValue <= 179) return HighNoonSun; // 155...179: 5400K
  if (dmxValue <= 204) return DirectSunlight; // 180...204: 6000K
  if (dmxValue <= 229) return OvercastSky; // 205...229: 7000K
  return ClearBlueSky; // 230...255: 20000K
}


uint8_t getEffectModeFromDmxValue(uint8_t dmxValue) {
  if (dmxValue <= 19) return FX_MODE_STATIC;
  if (dmxValue <= 22) return FX_MODE_BLINK;
  if (dmxValue <= 25) return FX_MODE_BREATH;
  if (dmxValue <= 28) return FX_MODE_COLOR_WIPE;
  if (dmxValue <= 31) return FX_MODE_COLOR_WIPE_INV;
  if (dmxValue <= 34) return FX_MODE_COLOR_WIPE_REV;
  if (dmxValue <= 37) return FX_MODE_COLOR_WIPE_REV_INV;
  if (dmxValue <= 40) return FX_MODE_COLOR_WIPE_RANDOM;
  if (dmxValue <= 43) return FX_MODE_RANDOM_COLOR;
  if (dmxValue <= 46) return FX_MODE_SINGLE_DYNAMIC;
  if (dmxValue <= 49) return FX_MODE_MULTI_DYNAMIC;
  if (dmxValue <= 52) return FX_MODE_RAINBOW;
  if (dmxValue <= 55) return FX_MODE_RAINBOW_CYCLE;
  if (dmxValue <= 58) return FX_MODE_SCAN;
  if (dmxValue <= 61) return FX_MODE_DUAL_SCAN;
  if (dmxValue <= 64) return FX_MODE_FADE;
  if (dmxValue <= 67) return FX_MODE_THEATER_CHASE;
  if (dmxValue <= 70) return FX_MODE_THEATER_CHASE_RAINBOW;
  if (dmxValue <= 73) return FX_MODE_RUNNING_LIGHTS;
  if (dmxValue <= 76) return FX_MODE_TWINKLE;
  if (dmxValue <= 79) return FX_MODE_TWINKLE_RANDOM;
  if (dmxValue <= 82) return FX_MODE_TWINKLE_FADE;
  if (dmxValue <= 85) return FX_MODE_TWINKLE_FADE_RANDOM;
  if (dmxValue <= 88) return FX_MODE_SPARKLE;
  if (dmxValue <= 91) return FX_MODE_FLASH_SPARKLE;
  if (dmxValue <= 94) return FX_MODE_HYPER_SPARKLE;
  if (dmxValue <= 97) return FX_MODE_STROBE;
  if (dmxValue <= 100) return FX_MODE_STROBE_RAINBOW;
  if (dmxValue <= 103) return FX_MODE_MULTI_STROBE;
  if (dmxValue <= 106) return FX_MODE_BLINK_RAINBOW;
  if (dmxValue <= 109) return FX_MODE_CHASE_WHITE;
  if (dmxValue <= 112) return FX_MODE_CHASE_COLOR;
  if (dmxValue <= 115) return FX_MODE_CHASE_RANDOM;
  if (dmxValue <= 118) return FX_MODE_CHASE_RAINBOW;
  if (dmxValue <= 121) return FX_MODE_CHASE_FLASH;
  if (dmxValue <= 124) return FX_MODE_CHASE_FLASH_RANDOM;
  if (dmxValue <= 127) return FX_MODE_CHASE_RAINBOW_WHITE;
  if (dmxValue <= 130) return FX_MODE_CHASE_BLACKOUT;
  if (dmxValue <= 133) return FX_MODE_CHASE_BLACKOUT_RAINBOW;
  if (dmxValue <= 136) return FX_MODE_COLOR_SWEEP_RANDOM;
  if (dmxValue <= 139) return FX_MODE_RUNNING_COLOR;
  if (dmxValue <= 142) return FX_MODE_RUNNING_RED_BLUE;
  if (dmxValue <= 145) return FX_MODE_RUNNING_RANDOM;
  if (dmxValue <= 148) return FX_MODE_LARSON_SCANNER;
  if (dmxValue <= 151) return FX_MODE_COMET;
  if (dmxValue <= 154) return FX_MODE_FIREWORKS;
  if (dmxValue <= 157) return FX_MODE_FIREWORKS_RANDOM;
  if (dmxValue <= 160) return FX_MODE_MERRY_CHRISTMAS;
  if (dmxValue <= 163) return FX_MODE_FIRE_FLICKER;
  if (dmxValue <= 166) return FX_MODE_FIRE_FLICKER_SOFT;
  if (dmxValue <= 169) return FX_MODE_FIRE_FLICKER_INTENSE;
  if (dmxValue <= 172) return FX_MODE_CIRCUS_COMBUSTUS;
  if (dmxValue <= 175) return FX_MODE_HALLOWEEN;
  if (dmxValue <= 178) return FX_MODE_BICOLOR_CHASE;
  if (dmxValue <= 181) return FX_MODE_TRICOLOR_CHASE;
  if (dmxValue <= 184) return FX_MODE_ICU;
  return FX_MODE_STATIC;
}


void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  if (universe != artNetUniverse) {
    return;
  }

  // don't call changeState(receiving) because we don't want it to set the LEDs
  currentState = receiving;
  lastArtNetPacketTime = millis();

  // DMX channel 1: Dimmer
  if (length > artNetStartAddress) {
    FastLED.setBrightness(data[artNetStartAddress]);
  }

  // DMX channel 2: Color Temperature
  if (length > artNetStartAddress + 1) {
    FastLED.setTemperature(getColorTemperatureFromDmxValue(data[artNetStartAddress + 1]));
  }

  // DMX channel 3: Pixel Control
  if (length > artNetStartAddress + 2) {
    isReversed = data[artNetStartAddress + 2] < 128;
    isHsv = (data[artNetStartAddress + 2] >= 64 && data[artNetStartAddress + 2] <= 127) || (data[artNetStartAddress + 2] >= 192);

    uint8_t effectOptions = isReversed ? REVERSE : NO_OPTIONS;
    if (effectSegment->options != effectOptions) {
      effectSegment->options = effectOptions;
      ws2812fx.trigger();
    }
  }

  // DMX channel 4: Auto Programs
  if (length > artNetStartAddress + 3) {
    if (data[artNetStartAddress + 3] >= 10) {
      uint8_t mode = getEffectModeFromDmxValue(data[artNetStartAddress + 3]);
      if (ws2812fx.getMode() != mode) {
        ws2812fx.setMode(mode);
        ws2812fx.trigger();
      }
      else if (!isEffectEnabled) {
        ws2812fx.trigger();
      }

      isEffectEnabled = true;
    }
    else {
      isEffectEnabled = false;
      ws2812fx.pause();
    }
  }

  // DMX channel 5: Program Speed
  if (length > artNetStartAddress + 4) {
    if (data[artNetStartAddress + 4] > 245) {
      if (ws2812fx.isRunning()) {
        ws2812fx.pause();
      }
    }
    else if (isEffectEnabled) {
      if (!ws2812fx.isRunning()) {
        ws2812fx.resume();
      }

      // (dmxValue + 10)^3 / 256 - 1
      // see https://www.wolframalpha.com/input/?i=plot+((x+%2B+10)%5E3+%2F+256+-+3)+from+x%3D0+to+245
      // and https://www.wolframalpha.com/input/?i=table+(Quotient%5B(x+%2B+10)%5E3,+256%5D+-+1)+from+x%3D0+to+245
      uint16_t speed = (data[artNetStartAddress + 4] + 10) * (data[artNetStartAddress + 4] + 10) * (data[artNetStartAddress + 4] + 10) / 256L - 1L;
      if (ws2812fx.getSpeed() != speed) {
        // set speed directly (instead of calling ws2812fx.setSpeed) to avoid resetting
        effectSegment->speed = speed;
        ws2812fx.trigger();
      }
    }
  }

  // DMX channels 6...125: single pixels or effect colors
  for (int i = 5; (i + 3) < length; i += 4) {
    int ledIndex = (i - 5) / 4;
    if (ledIndex >= NUM_LEDS) {
      break;
    }

    CRGB *pixel;

    if (isEffectEnabled) {
      if (ledIndex >= NUM_EFFECT_COLORS) {
        break;
      }

      pixel = &effectColors[ledIndex];
    }
    else if (isReversed) {
      pixel = &leds[NUM_LEDS - ledIndex - 1];
    }
    else {
      pixel = &leds[ledIndex];
    }

    if (isHsv) {
      pixel->setHSV(data[artNetStartAddress + i + 1], data[artNetStartAddress + i + 2], data[artNetStartAddress + i + 3]);
    }
    else {
      pixel->setRGB(data[artNetStartAddress + i + 1], data[artNetStartAddress + i + 2], data[artNetStartAddress + i + 3]);
    }

    // brightness
    pixel->nscale8(255 - data[artNetStartAddress + i]);
  }

  if (isEffectEnabled) {
    for (int i = 0; i < NUM_EFFECT_COLORS; i++) {
      uint32_t newColor = ((uint32_t)effectColors[i].r << 16) | ((uint32_t)effectColors[i].g << 8) | (effectColors[i].b);
      effectSegment->colors[i] = newColor;
    }
  }
}


void setup() {
  Serial.begin(115200); // for USB communication
  Serial1.begin(600, SERIAL_5E2, RX_PIN, TX_PIN); // for ESP32-to-ESP32 communication

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  // reset any leftover LED assignments from before the reboot
  fill_solid(leds, NUM_LEDS, CRGB(0, 0, 0));

  /* Serial.println("Booting up, trying to find out pixel tube number in line...");
  pixelTubeNumberInLine = getPixelTubeNumberInLine();
  Serial.printf("I am pixel tube number %d in line.\n", pixelTubeNumberInLine);

  // only 6 Pixel Tubes are allowed in one line
  if (pixelTubeNumberInLine > 6) {
    changeState(tooManyPixelTubesInLine);
    pixelTubeNumberInLine = 99;
    return;
  } */

  initTest();
  iot.begin("LEDs4TheWin!");

  if (iot.wifi.getOperationMode() != WifiControl::Mode::client) {
    changeState(unconfigured);
  }
  else {
    changeState(noWiFi);

    WifiControl::onConnectCallback = []() {
      changeState(noArtNet);

      artNetUniverse = iot.artNetUniverse;
      artNetStartAddress = iot.artNetStartAddress - 1;

      IPAddress nodeIp = iot.wifi.getIP();
      IPAddress subnetMask = iot.wifi.subnetMask();
      IPAddress broadcastIp = IPAddress(nodeIp | ~subnetMask);

      String shortName = "PixTube" + String(iot.pixelTubeNumber);
      String longName = "Gruft Pixel Tube " + String(iot.pixelTubeNumber);

      artnet.setArtPollReplyInformation(nodeIp, broadcastIp, shortName, longName);
    };

    WifiControl::onDisconnectCallback = []() {
      if (currentState == noArtNet) {
        // directly change back to noWiFi
        changeState(noWiFi);
      }

      // if we are in receiving state, the state change to noWiFi will be
      // initiated after one minute by the timer, so don't do anything here
    };

    ws2812fx.init();
    ws2812fx.setCustomShow(ws2812fxShowFunction);
    effectSegment = ws2812fx.getSegment();
    ws2812fx.setBrightness(255);
    ws2812fx.start();
    ws2812fx.pause();

    artnet.setArtDmxCallback(onDmxFrame);
    artnet.begin();
  }
}


uint16_t artNetReturnCode;

void loop() {
  // Serial1.write(pixelTubeNumberInLine);

  switch (currentState) {
    case receiving:
      artNetReturnCode = artnet.read();
      ws2812fx.service();
      FastLED.show();

      if (artNetReturnCode != 0) {
        // valid Art-Net packet
        return;
      }

      if (millis() - lastArtNetPacketTime > 60000) {
        if (iot.wifi.status() == WL_CONNECTED) {
          changeState(noArtNet);
        }
        else {
          changeState(noWiFi);
        }
      }
      return;

    case tooManyPixelTubesInLine:
      showError(255, 0, 0);
      return;

    case unconfigured:
      showError(255, 255, 255);
      return;

    case noWiFi:
      showError(0, 255, 0);
      return;

    case noArtNet:
      showError(0, 0, 255);
      artnet.read(); // try to read an Art-Net packet anyway
      FastLED.show();
      return;
  }
}
