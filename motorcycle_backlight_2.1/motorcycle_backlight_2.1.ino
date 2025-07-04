#include <Adafruit_NeoPixel.h>

#define LED_PIN     6
#define LED_COUNT   16
#define BRIGHTNESS  80

#define LEFT_PIN    2
#define BRAKE_PIN   3
#define RIGHT_PIN   4

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Inputs
bool leftSignal = false;
bool rightSignal = false;
bool brakeSignal = false;
// Blinker state
unsigned long lastStepTime = 0;
int blinkerIndex = 0;
unsigned long blinkerSpeed = 100; // speed of stacking animation

// Brake flashing
bool brakeStarted = false;
unsigned long brakeStartTime = 0;

void setup() {
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(BRAKE_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);

  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();
}

void loop() {
  // Read input states (active low)
  leftSignal = digitalRead(LEFT_PIN) == LOW;
  brakeSignal = digitalRead(BRAKE_PIN) == LOW;
  rightSignal = digitalRead(RIGHT_PIN) == LOW;

  // Reset brake state
  if (!brakeSignal) {
    brakeStarted = false;
  }

  // Step 1: Set background dim red everywhere EXCEPT active blinker zones
  drawTailLight();

  // Step 2: Handle brake light
  if (brakeSignal) {
    brakeFull();
  }

  // Step 3: Handle indicators (stacking fill style)
  if (leftSignal) {
    runningLeftBlinker();
  }

  if (rightSignal) {
    runningRightBlinker();
  }

  // Step 4: Show updated strip
  strip.show();
  delay(blinkerSpeed);  // keep smooth animation
}

// --- Tail light dim red, skip active blinker zones ---
void drawTailLight() {
  uint32_t dimRed = strip.Color(30, 0, 0); // Dim red

  for (int i = 0; i < LED_COUNT; i++) {
    if ((leftSignal && i >= 0 && i < 8) || (rightSignal && i >= 8 && i < 16)) {
      continue; // Skip blinker zones when active
    }
    strip.setPixelColor(i, dimRed);
  }
}

// --- Brake: brighten only zones not occupied by blinkers ---
void brakeFull() {
  if (!brakeStarted) {
    brakeStarted = true;
    brakeStartTime = millis();
  }

  unsigned long elapsed = millis() - brakeStartTime;
  bool flash = (elapsed < 1000) ? ((elapsed / 100) % 2 == 0) : true;

  uint32_t brakeColor = flash ? strip.Color(255, 0, 0) : strip.Color(30, 0, 0); // full red or tail red

  if (leftSignal && !rightSignal) {
    for (int i = 8; i < 16; i++) {
      strip.setPixelColor(i, brakeColor);
    }
  } else if (rightSignal && !leftSignal) {
    for (int i = 0; i < 8; i++) {
      strip.setPixelColor(i, brakeColor);
    }
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, brakeColor);
    }
  }
}

// --- Left indicator stacking (LEDs 0–7) ---
void runningLeftBlinker() {
  unsigned long now = millis();
  if (now - lastStepTime >= blinkerSpeed) {
    for (int i = 0; i <= blinkerIndex; i++) {
      int led = 7 - i;
      if (led >= 0) {
        strip.setPixelColor(led, strip.Color(255, 120, 0)); // Amber
      }
    }

    blinkerIndex++;
    if (blinkerIndex >= 8) {
      blinkerIndex = 0;
      // Clear blinker zone for restart
      for (int i = 0; i < 8; i++) {
        strip.setPixelColor(i, 0); // Will be filled by tail light in next loop
      }
    }

    lastStepTime = now;
  }
}

// --- Right indicator stacking (LEDs 8–15) ---
void runningRightBlinker() {
  unsigned long now = millis();
  if (now - lastStepTime >= blinkerSpeed) {
    for (int i = 0; i <= blinkerIndex; i++) {
      int led = 8 + i;
      if (led < 16) {
        strip.setPixelColor(led, strip.Color(255, 120, 0)); // Amber
      }
    }

    blinkerIndex++;
    if (blinkerIndex >= 8) {
      blinkerIndex = 0;
      // Clear blinker zone for restart
      for (int i = 8; i < 16; i++) {
        strip.setPixelColor(i, 0); // Will be refilled by tail light in next loop
      }
    }

    lastStepTime = now;
  }
}
