#include <Wire.h>
#include <Adafruit_NeoPixel.h>

#define NUM_PIXELS 16
#define NUM_PAIRS 8
#define INPUT_PIN 5
#define BUTTON_PIN 6
#define TOGGLE_MODE_DELAY 250
#define BRIGHTNESS 25

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, INPUT_PIN, NEO_GRB + NEO_KHZ800);

/**
 * UTIL
 */

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte wheelPos) {
  if (wheelPos < 85) {
   return strip.Color(wheelPos * 3, 255 - wheelPos * 3, 0);
  } else if (wheelPos < 170) {
   wheelPos -= 85;
   return strip.Color(255 - wheelPos * 3, 0, wheelPos * 3);
  } else {
   wheelPos -= 170;
   return strip.Color(0, wheelPos * 3, 255 - wheelPos * 3);
  }
}

int isButtonPressed() {
  return digitalRead(BUTTON_PIN) == 1;
}

/**
 * PATTERN 1
 * Alternating
 * Blinks patterns of lights in pairs circling around the band
*/

#define ALTERNATE_BLINK_DELAY 50

int alternating_pinPairs[][NUM_PAIRS] = {
  {0, 8},
  {1, 9},
  {2, 10},
  {3, 11},
  {4, 12},
  {5, 13},
  {6, 14},
  {7, 15}
};

uint8_t alternating_currentPair = 0;

void runPatternAlternating() {
  if (++alternating_currentPair >= NUM_PAIRS) alternating_currentPair = 0;
  alternating_turnOnPair(alternating_pinPairs[alternating_currentPair]);
  delay(ALTERNATE_BLINK_DELAY);
  alternating_turnOffPair(alternating_pinPairs[alternating_currentPair]);
}

void alternating_turnOnPair(int pair[]) {
  uint32_t color = strip.Color(255, 255, 255);
  strip.setPixelColor(pair[0], color);
  strip.setPixelColor(pair[1], color);
  strip.show();
}

void alternating_turnOffPair(int pair[]) {
  uint32_t color = strip.Color(0, 0, 0);
  strip.setPixelColor(pair[0], color);
  strip.setPixelColor(pair[1], color);
  strip.show();
}

/**
 * PATTERN 2
 * Green/Pink
 * Blinks in two colors in a ring
 */

#define GREEN_PINK_BLINK_DELAY 5
#define GREEN_PINK_NUM_COLORS 10

int greenPink_currentPixel = 0;
int greenPink_direction;
int greenPink_colorIndex = 0;

// Here is where you can put in your favorite colors that will appear!
// just add new {nnn, nnn, nnn}, lines. They will be picked out randomly
//                                  R   G   B
uint8_t colorList[][GREEN_PINK_NUM_COLORS] = {
  {0, 255, 0},
  {255, 0, 255},
  {0, 255, 0},
  {255, 0, 255},
  {0, 255, 0},
  {255, 0, 255},
  {0, 255, 0},
  {255, 0, 255},
  {0, 255, 0},
  {255, 0, 255}
};

void runGreenPinkPattern() {
  if (++greenPink_colorIndex == GREEN_PINK_NUM_COLORS) {
    greenPink_colorIndex = 0;
  }

  int red = colorList[greenPink_colorIndex][0];
  int green = colorList[greenPink_colorIndex][1];
  int blue = colorList[greenPink_colorIndex][2];

  // Bounds detection
  if (greenPink_currentPixel == 0) {
    greenPink_direction = 1;
  } else if (greenPink_currentPixel == NUM_PIXELS) {
    greenPink_direction = -1;
  }

  // Advancing the pixel
  greenPink_currentPixel = greenPink_currentPixel + (1 * greenPink_direction);

  // now we will 'fade' it in 5 steps
  for (int x = 0; x < 5; x++) {
    int r = red * (x+1); r /= 5;
    int g = green * (x+1); g /= 5;
    int b = blue * (x+1); b /= 5;

    strip.setPixelColor(greenPink_currentPixel, strip.Color(r, g, b));
    strip.show();
    delay(GREEN_PINK_BLINK_DELAY);
  }

  // & fade out in 5 steps
  for (int x = 5; x >= 0; x--) {
    int r = red * x; r /= 5;
    int g = green * x; g /= 5;
    int b = blue * x; b /= 5;

    strip.setPixelColor(greenPink_currentPixel, strip.Color(r, g, b));
    strip.show();
    delay(GREEN_PINK_BLINK_DELAY);
  }
}

/**
 * PATTERN 3
 * Rainbow
 * Fades lights across the rainbow
 */

#define RAINBOW_CYCLE_DELAY 50

byte rainbow_currentColor = 0;

void runRainbowPattern() {
  uint8_t i;
  for (i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, Wheel(rainbow_currentColor));
    strip.show();
  }
  if (++rainbow_currentColor >= 255) rainbow_currentColor = 0;
  delay(RAINBOW_CYCLE_DELAY);
}

/**
 * CONTROL
 */

int currentPattern = 0;

#define NUM_PATTERNS 3

char *PATTERN_NAMES[NUM_PATTERNS] = {
  "rainbow",
  "greenPink",
  "alternating"
};

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(BRIGHTNESS);
  Serial.begin(9600);
}

void loop() {
  if (isButtonPressed()) {
    currentPattern = (currentPattern + 1) % NUM_PATTERNS;
    strip.clear();
    strip.show();
    delay(TOGGLE_MODE_DELAY);
  }

  switch (currentPattern) {
    case 0:
      runRainbowPattern();
      break;
    case 1:
      runGreenPinkPattern();
      break;
    case 2:
      runPatternAlternating();
      break;
  }
}