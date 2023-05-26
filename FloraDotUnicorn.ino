#include <Adafruit_NeoPixel.h>

#define RANDOM_SEED_PIN 6
#define PIN1 8
#define ONBOARD_SIZE 1
#define PIN2 12
#define CHAIN2_SIZE 60
#define NUM_CHAINS 2

Adafruit_NeoPixel onboard = Adafruit_NeoPixel(ONBOARD_SIZE, PIN1);
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(CHAIN2_SIZE, PIN2, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel strips[NUM_CHAINS];

void setup() {

  // Try to inject some true randomness, but reading an unused PIN sucks. I've observed it return the same value each time.
  // See this stackoverflow for some ideas to increase the entropy
  // https://arduino.stackexchange.com/questions/22070/powering-from-3v-coin-cell-to-vcc-using-i-o-pin-as-supply-gnd/22081#22081
  uint8_t seed = analogRead(RANDOM_SEED_PIN);

  Serial.begin(9600);

  // Required so serial monitor can be initialized before the first message is sent.
  delay(1000);
  Serial.print("Random Seed: ");
  Serial.println(seed);

  randomSeed(seed);

  strips[0] = onboard;
  strips[1] = strip1;

  onboard.begin();
  onboard.setBrightness(25);
  onboard.show();  // Initialize all pixels to 'off'

  strip1.begin();
  strip1.setBrightness(100);
  strip1.show();  // Initialize all pixels to 'off'
}

void loop() {
  smoothTwinkle2(1);
}

void demoSequence() {
  demoMarker();

  rainbowCycle(25, 3);

  demoMarker();

  randomTwinkle(25);

  demoMarker();

  smoothTwinkle1(25);

  demoMarker();

  smoothTwinkle2(5);

  demoMarker();

  demoMarker();
}

void demoMarker() {
  strip1.clear();
  strip1.show();
  delay(1000);

  int i;
  for (i = 0; i < 3; i++) {
    flashWhite();
  }
}

void flashWhite() {
  int i;
  for (i = 0; i < CHAIN2_SIZE; i++) {
    strip1.setPixelColor(i, 128, 128, 128);
  }
  strip1.show();
  delay(200);
  strip1.clear();
  strip1.show();
  delay(250);
}

void randomTwinkle(int reps) {
  uint8_t i, r;
  for (r = 0; r < reps; r++) {
    for (i = 0; i < CHAIN2_SIZE / 3; i++) {
      strip1.setPixelColor(random(CHAIN2_SIZE), strip1.Color(random(0, 256), random(0, 256), random(0, 256)));
      strip1.show();
    }
    delay(500);
  }
}

void smoothTwinkle1(uint8_t reps) {
  uint8_t num_pix = CHAIN2_SIZE / 3, r, p;
  uint8_t pixels[num_pix];
  uint8_t candidate;

  for (r = 0; r < reps; r++) {
    p = 0;
    while (p < num_pix) {
      candidate = random(0, CHAIN2_SIZE);
      if (!contains(pixels, num_pix, candidate)) {
        pixels[p] = candidate;
        p++;
      }
    }

    for (p = 0; p < num_pix; p++) {
      strip1.setPixelColor(pixels[p], strip1.Color(random(0, 256), random(0, 256), random(0, 256)));
    }

    strip1.show();
    delay(1000);
  }
}

void smoothTwinkle2(uint8_t reps) {
  // Function config
  // TODO: refactor all uint8_t to int
  // TODO: use const where appropriate

  uint8_t red_reduction = 4, blue_reduction = 1, green_reduction = 1;

  // variables
  uint8_t brightness_steps = 100;
  uint8_t bs = brightness_steps;
  uint8_t num_pix = CHAIN2_SIZE / 3, r, p, i, candidate;
  uint8_t pixels[num_pix];
  uint8_t brightness[num_pix], reds[num_pix], greens[num_pix], blues[num_pix];

  // choose pixels to participate in cycle
  p = 0;
  while (p < num_pix) {
    candidate = random(0, CHAIN2_SIZE);
    if (!contains(pixels, num_pix, candidate)) {
      pixels[p] = candidate;
      p++;
    }
  }

  // initialize cycle
  for (p = 0; p < num_pix; p++) {
    brightness[p] = random(1, brightness_steps);
    reds[p] = random(0, 256);
    greens[p] = random(0, 256);
    blues[p] = random(0, 256);
  }

  int pb;  // pixel specific brightness factor
  for (i = 0; i < brightness_steps * reps; i++) {
    for (p = 0; p < num_pix; p++) {
      pb = brightness[p];
      strip1.setPixelColor(pixels[p], reds[p] * pb / bs / red_reduction, greens[p] * pb / bs / blue_reduction, blues[p] * pb / bs / green_reduction);
      if (pb == 0) {
        // transition to new active pixel
        candidate = random(0, CHAIN2_SIZE);
        while (contains(pixels, num_pix, candidate)) {
          candidate = random(0, CHAIN2_SIZE);
        }
        pixels[p] = candidate;
      }

      // set next brightness
      if (pb == 0) {
        brightness[p] = 1;
      } else if (pb == 99) {
        brightness[p] = 98;
      } else if (pb % 2 == 0) {
        brightness[p] = pb - 2;
      } else {
        brightness[p] = pb + 2;
      }
    }

    strip1.show();
    delay(20);
  }
}


bool contains(uint8_t* array, uint8_t size, uint8_t val) {
  uint8_t i;
  for (i = 0; i < size; i++) {
    if ((array[i]) == val) {
      return true;
    }
  }

  return false;
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait, uint8_t reps) {
  uint16_t i, j, s;

  for (j = 0; j < 256 * reps; j++) {  // 5 cycles of all colors on wheel
    for (s = 0; s < NUM_CHAINS; s++) {
      for (i = 0; i < strips[s].numPixels(); i++) {
        strips[s].setPixelColor(i, Wheel(((i * 256 / strips[s].numPixels()) + j) & 255) / 3);
      }
      strips[s].show();
    }
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip1.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip1.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip1.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}