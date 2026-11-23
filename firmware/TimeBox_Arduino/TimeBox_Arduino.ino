// TimeBox for Arduino Uno: choose a duration, close the lid, then press START.
// No lid sensor. Reset/power loss clears a running countdown.
// Change these angles after testing the servo arm with the lid open.
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

const uint8_t START_PIN = 4, BUZZER_PIN = 8, SERVO_PIN = 9;
const uint8_t KEYPAD_PIN = A0;
const uint8_t UNLOCK_ANGLE = 15, LOCK_ANGLE = 85;
const uint8_t BUZZER_ON = HIGH, BUZZER_OFF = LOW;
const uint32_t PRESETS[] = {30UL, 900UL, 1800UL, 3600UL, 5400UL, 7200UL, 14400UL};
const uint8_t PRESET_COUNT = sizeof(PRESETS) / sizeof(PRESETS[0]);
const int KEY_TOLERANCE = 14;

LiquidCrystal_I2C screen27(0x27, 16, 2), screen3F(0x3F, 16, 2);
LiquidCrystal_I2C *lcd = &screen27;
Servo latch;

struct KeySettings {
  int16_t idle, plus, minus;
};
KeySettings keys;

enum : uint8_t { READY, LOCKED, DONE };
enum : uint8_t { LEARN_IDLE, LEARN_PLUS, RELEASE_PLUS, LEARN_MINUS, RELEASE_MINUS, KEYS_READY };
uint8_t state = READY, learning = LEARN_IDLE, selected = 3; // Default: one hour
uint32_t startedAt = 0, beepAt = 0, lastScreen = 0;
bool beeping = false, screenDirty = true;

int startIdle = LOW, startRaw = LOW, startStable = LOW;
uint32_t startChanged = 0;
int keyCandidate = 0, keyStable = 0;
uint32_t keyChanged = 0;
bool keyArmed = false;

bool hasI2C(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void printLine(uint8_t row, const char *s) {
  lcd->setCursor(0, row);
  bool ended = false;
  for (uint8_t i = 0; i < 16; ++i) {
    if (!ended && s[i] == '\0') ended = true;
    lcd->write(ended ? ' ' : s[i]);
  }
}

void showScreen(uint32_t now) {
  switch (learning) {
    case LEARN_IDLE: printLine(0, "Keypad setup"); printLine(1, "Release all keys"); return;
    case LEARN_PLUS: printLine(0, "Hold key for +"); printLine(1, "Use keypad"); return;
    case LEARN_MINUS: printLine(0, "Hold key for -"); printLine(1, "Different key"); return;
    case RELEASE_PLUS:
    case RELEASE_MINUS: printLine(0, "Key saved"); printLine(1, "Release key"); return;
  }

  if (state == DONE) {
    printLine(0, "TIME IS UP");
    printLine(1, "UNLOCKED");
    return;
  }

  uint32_t seconds = PRESETS[selected];
  if (state == LOCKED) {
    uint32_t elapsed = now - startedAt;
    uint32_t total = seconds * 1000UL;
    seconds = elapsed >= total ? 0 : (total - elapsed + 999UL) / 1000UL;
  }
  char line[24];
  snprintf(line, sizeof(line), "%s %02u:%02u:%02u",
           state == LOCKED ? "LEFT" : "SET ",
           (unsigned)(seconds / 3600UL), (unsigned)((seconds / 60UL) % 60UL),
           (unsigned)(seconds % 60UL));
  printLine(0, line);
  printLine(1, state == LOCKED ? "LOCKED" : selected == 0 ? "DEMO: START" : "CLOSE LID: START");
}

int readKeypad() {
  int total = 0;
  for (uint8_t i = 0; i < 5; ++i) total += analogRead(KEYPAD_PIN);
  return total / 5;
}

bool farApart(int a, int b) {
  return abs(a - b) > 2 * KEY_TOLERANCE + 6;
}

void pollCalibration(uint32_t now) {
  if (uint32_t(now - keyChanged) < 250UL) return;
  switch (learning) {
    case LEARN_IDLE:
      keys.idle = keyStable;
      learning = LEARN_PLUS;
      break;
    case LEARN_PLUS:
      if (!farApart(keyStable, keys.idle)) return;
      keys.plus = keyStable;
      learning = RELEASE_PLUS;
      break;
    case RELEASE_PLUS:
      if (abs(keyStable - keys.idle) > KEY_TOLERANCE) return;
      learning = LEARN_MINUS;
      break;
    case LEARN_MINUS:
      if (!farApart(keyStable, keys.idle) || !farApart(keyStable, keys.plus)) return;
      keys.minus = keyStable;
      learning = RELEASE_MINUS;
      break;
    case RELEASE_MINUS:
      if (abs(keyStable - keys.idle) > KEY_TOLERANCE) return;
      learning = KEYS_READY;
      keyArmed = true;
      break;
    default: return;
  }
  screenDirty = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(START_PIN, INPUT); // Three-pin button module drives S; learn its idle level at boot.
  startIdle = startRaw = startStable = digitalRead(START_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, BUZZER_OFF);
  pinMode(LED_BUILTIN, OUTPUT);
  latch.write(UNLOCK_ANGLE);
  latch.attach(SERVO_PIN);
  Wire.begin();
  if (!hasI2C(0x27) && hasI2C(0x3F)) lcd = &screen3F;
  lcd->init();
  lcd->backlight();
  keyCandidate = keyStable = readKeypad();
  keyChanged = startChanged = millis();
  Serial.println(F("TimeBox. Release buttons at startup. Send c to relearn keypad."));
}

void loop() {
  const uint32_t now = millis();
  const int reading = digitalRead(START_PIN);
  if (reading != startRaw) {
    startRaw = reading;
    startChanged = now;
  }
  bool startPressed = false;
  if (startStable != startRaw && uint32_t(now - startChanged) >= 35UL) {
    startStable = startRaw;
    startPressed = startStable != startIdle;
  }

  int value = readKeypad();
  if (abs(value - keyCandidate) > 5) {
    keyCandidate = value;
    keyChanged = now;
  }
  if (uint32_t(now - keyChanged) >= 60UL) keyStable = keyCandidate;

  if (Serial.available() && Serial.read() == 'c' && state != LOCKED) {
    learning = LEARN_IDLE;
    keyChanged = now;
    screenDirty = true;
  }

  if (beeping && uint32_t(now - beepAt) >= 250UL) {
    digitalWrite(BUZZER_PIN, BUZZER_OFF);
    beeping = false;
  }

  if (learning != KEYS_READY) {
    pollCalibration(now);
  } else if (state == LOCKED) {
    if (uint32_t(now - startedAt) >= PRESETS[selected] * 1000UL) {
      state = DONE;
      latch.write(UNLOCK_ANGLE);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(BUZZER_PIN, BUZZER_ON);
      beepAt = now;
      beeping = true;
      screenDirty = true;
    }
  } else {
    if (abs(keyStable - keys.idle) <= KEY_TOLERANCE) {
      keyArmed = true;
    } else if (keyArmed && uint32_t(now - keyChanged) >= 60UL) {
      if (abs(keyStable - keys.plus) <= KEY_TOLERANCE) {
        if (selected + 1 < PRESET_COUNT) ++selected;
        state = READY;
        screenDirty = true;
      } else if (abs(keyStable - keys.minus) <= KEY_TOLERANCE) {
        if (selected > 0) --selected;
        state = READY;
        screenDirty = true;
      }
      keyArmed = false;
    }
    if (startPressed) {
      startedAt = now;
      state = LOCKED;
      keyArmed = false;
      latch.write(LOCK_ANGLE);
      digitalWrite(LED_BUILTIN, HIGH);
      screenDirty = true;
    }
  }

  if (screenDirty || (state == LOCKED && uint32_t(now - lastScreen) >= 200UL)) {
    screenDirty = false;
    lastScreen = now;
    showScreen(now);
  }
}
