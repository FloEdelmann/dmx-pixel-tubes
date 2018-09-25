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

CRGB getColorMacroFromDmxValue(uint8_t dmxValue) {
  if (dmxValue == 116) return CRGB::Black;
  if (dmxValue == 117) return CRGB::DimGrey;
  if (dmxValue == 118) return CRGB::Grey;
  if (dmxValue == 119) return CRGB::DarkGrey;
  if (dmxValue == 120) return CRGB::Silver;
  if (dmxValue == 121) return CRGB::LightGrey;
  if (dmxValue == 122) return CRGB::Gainsboro;
  if (dmxValue == 123) return CRGB::DarkSlateGrey;
  if (dmxValue == 124) return CRGB::LightSlateGrey;
  if (dmxValue == 125) return CRGB::SlateGrey;
  if (dmxValue == 126) return CRGB::SteelBlue;
  if (dmxValue == 127) return CRGB::SlateBlue;
  if (dmxValue == 128) return CRGB::DarkSlateBlue;
  if (dmxValue == 129) return CRGB::MidnightBlue;
  if (dmxValue == 130) return CRGB::Navy;
  if (dmxValue == 131) return CRGB::DarkBlue;
  if (dmxValue == 132) return CRGB::MediumBlue;
  if (dmxValue == 133) return CRGB::Blue;
  if (dmxValue == 134) return CRGB::RoyalBlue;
  if (dmxValue == 135) return CRGB::MediumSlateBlue;
  if (dmxValue == 136) return CRGB::DodgerBlue;
  if (dmxValue == 137) return CRGB::CornflowerBlue;
  if (dmxValue == 138) return CRGB::DeepSkyBlue;
  if (dmxValue == 139) return CRGB::SkyBlue;
  if (dmxValue == 140) return CRGB::LightSkyBlue;
  if (dmxValue == 141) return CRGB::LightSteelBlue;
  if (dmxValue == 142) return CRGB::PowderBlue;
  if (dmxValue == 143) return CRGB::LightBlue;
  if (dmxValue == 144) return CRGB::PaleTurquoise;
  if (dmxValue == 145) return CRGB::LightCyan;
  if (dmxValue == 146) return CRGB::AliceBlue;
  if (dmxValue == 147) return CRGB::Azure;
  if (dmxValue == 148) return CRGB::Honeydew;
  if (dmxValue == 149) return CRGB::MintCream;
  if (dmxValue == 150) return CRGB::Aquamarine;
  if (dmxValue == 151) return CRGB::Cyan;
  if (dmxValue == 152) return CRGB::MediumTurquoise;
  if (dmxValue == 153) return CRGB::Turquoise;
  if (dmxValue == 154) return CRGB::LightSeaGreen;
  if (dmxValue == 155) return CRGB::DarkTurquoise;
  if (dmxValue == 156) return CRGB::MediumAquamarine;
  if (dmxValue == 157) return CRGB::CadetBlue;
  if (dmxValue == 158) return CRGB::DarkCyan;
  if (dmxValue == 159) return CRGB::Teal;
  if (dmxValue == 160) return CRGB::DarkOliveGreen;
  if (dmxValue == 161) return CRGB::Olive;
  if (dmxValue == 162) return CRGB::OliveDrab;
  if (dmxValue == 163) return CRGB::DarkGreen;
  if (dmxValue == 164) return CRGB::Green;
  if (dmxValue == 165) return CRGB::ForestGreen;
  if (dmxValue == 166) return CRGB::SeaGreen;
  if (dmxValue == 167) return CRGB::MediumSeaGreen;
  if (dmxValue == 168) return CRGB::DarkSeaGreen;
  if (dmxValue == 169) return CRGB::YellowGreen;
  if (dmxValue == 170) return CRGB::LimeGreen;
  if (dmxValue == 171) return CRGB::MediumSpringGreen;
  if (dmxValue == 172) return CRGB::SpringGreen;
  if (dmxValue == 173) return CRGB::Lime;
  if (dmxValue == 174) return CRGB::LawnGreen;
  if (dmxValue == 175) return CRGB::Chartreuse;
  if (dmxValue == 176) return CRGB::GreenYellow;
  if (dmxValue == 177) return CRGB::LightGreen;
  if (dmxValue == 178) return CRGB::PaleGreen;
  if (dmxValue == 179) return CRGB::Yellow;
  if (dmxValue == 180) return CRGB::Gold;
  if (dmxValue == 181) return CRGB::Khaki;
  if (dmxValue == 182) return CRGB::PaleGoldenrod;
  if (dmxValue == 183) return CRGB::LemonChiffon;
  if (dmxValue == 184) return CRGB::LightGoldenrodYellow;
  if (dmxValue == 185) return CRGB::LightYellow;
  if (dmxValue == 186) return CRGB::Cornsilk;
  if (dmxValue == 187) return CRGB::OldLace;
  if (dmxValue == 188) return CRGB::Beige;
  if (dmxValue == 189) return CRGB::AntiqueWhite;
  if (dmxValue == 190) return CRGB::Linen;
  if (dmxValue == 191) return CRGB::BlanchedAlmond;
  if (dmxValue == 192) return CRGB::PapayaWhip;
  if (dmxValue == 193) return CRGB::Wheat;
  if (dmxValue == 194) return CRGB::Bisque;
  if (dmxValue == 195) return CRGB::NavajoWhite;
  if (dmxValue == 196) return CRGB::Moccasin;
  if (dmxValue == 197) return CRGB::Tan;
  if (dmxValue == 198) return CRGB::DarkKhaki;
  if (dmxValue == 199) return CRGB::DarkGoldenrod;
  if (dmxValue == 200) return CRGB::Peru;
  if (dmxValue == 201) return CRGB::Chocolate;
  if (dmxValue == 202) return CRGB::Goldenrod;
  if (dmxValue == 203) return CRGB::BurlyWood;
  if (dmxValue == 204) return CRGB::SandyBrown;
  if (dmxValue == 205) return CRGB::Orange;
  if (dmxValue == 206) return CRGB::DarkOrange;
  if (dmxValue == 207) return CRGB::Coral;
  if (dmxValue == 208) return CRGB::LightSalmon;
  if (dmxValue == 209) return CRGB::Salmon;
  if (dmxValue == 210) return CRGB::DarkSalmon;
  if (dmxValue == 211) return CRGB::LightCoral;
  if (dmxValue == 212) return CRGB::Tomato;
  if (dmxValue == 213) return CRGB::OrangeRed;
  if (dmxValue == 214) return CRGB::Red;
  if (dmxValue == 215) return CRGB::Crimson;
  if (dmxValue == 216) return CRGB::IndianRed;
  if (dmxValue == 217) return CRGB::Plaid;
  if (dmxValue == 218) return CRGB::Sienna;
  if (dmxValue == 219) return CRGB::FireBrick;
  if (dmxValue == 220) return CRGB::SaddleBrown;
  if (dmxValue == 221) return CRGB::DarkRed;
  if (dmxValue == 222) return CRGB::Maroon;
  if (dmxValue == 223) return CRGB::Brown;
  if (dmxValue == 224) return CRGB::RosyBrown;
  if (dmxValue == 225) return CRGB::PeachPuff;
  if (dmxValue == 226) return CRGB::LavenderBlush;
  if (dmxValue == 227) return CRGB::MistyRose;
  if (dmxValue == 228) return CRGB::Pink;
  if (dmxValue == 229) return CRGB::LightPink;
  if (dmxValue == 230) return CRGB::HotPink;
  if (dmxValue == 231) return CRGB::DeepPink;
  if (dmxValue == 232) return CRGB::Magenta;
  if (dmxValue == 233) return CRGB::PaleVioletRed;
  if (dmxValue == 234) return CRGB::MediumVioletRed;
  if (dmxValue == 235) return CRGB::Orchid;
  if (dmxValue == 236) return CRGB::Plum;
  if (dmxValue == 237) return CRGB::MediumOrchid;
  if (dmxValue == 238) return CRGB::DarkOrchid;
  if (dmxValue == 239) return CRGB::Amethyst;
  if (dmxValue == 240) return CRGB::BlueViolet;
  if (dmxValue == 241) return CRGB::DarkViolet;
  if (dmxValue == 242) return CRGB::Indigo;
  if (dmxValue == 243) return CRGB::Purple;
  if (dmxValue == 244) return CRGB::DarkMagenta;
  if (dmxValue == 245) return CRGB::MediumPurple;
  if (dmxValue == 246) return CRGB::Violet;
  if (dmxValue == 247) return CRGB::Thistle;
  if (dmxValue == 248) return CRGB::Lavender;
  if (dmxValue == 249) return CRGB::Seashell;
  if (dmxValue == 250) return CRGB::FloralWhite;
  if (dmxValue == 251) return CRGB::Ivory;
  if (dmxValue == 252) return CRGB::WhiteSmoke;
  if (dmxValue == 253) return CRGB::GhostWhite;
  if (dmxValue == 254) return CRGB::Snow;
  if (dmxValue == 255) return CRGB::White;
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
  if (length >= 2) {
    FastLED.setTemperature(getColorTemperatureFromDmxValue(data[1]));
  }

  // DMX channel 3: Color Macros
  if (length >= 3 && data[2] >= 116) {
    fill_solid(leds, NUM_LEDS, getColorMacroFromDmxValue(data[2]));
    FastLED.show();
    return;
  }

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
