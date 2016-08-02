#include <Bounce2.h>

#include <RobbieComms.h>

#define DEVICE_ID 0x04

#define SHIFTOUT_DATA 3
#define SHIFTOUT_CLOCK 4
#define SHIFTOUT_LATCH 5

#define SWITCH_1_PIN A0
#define SWITCH_2_PIN A1
#define SWITCH_3_PIN A2
#define SWITCH_4_PIN A3

#define STORE_PIN A4

#define LED_START 2

uint8_t targetValue = 0;
uint32_t lastDisplay = 0;
uint32_t lastCheck = 0;

uint8_t completeCount = 0;
uint8_t flashOn = false;
Bounce storeButton;
RobbieComms comms(DEVICE_ID);

void setup() {
  Serial.begin(9600);
  comms.begin();
  comms.setCallBacks(newGame, enableGame, 0);
  
  pinMode(SWITCH_1_PIN, INPUT_PULLUP);
  pinMode(SWITCH_2_PIN, INPUT_PULLUP);
  pinMode(SWITCH_3_PIN, INPUT_PULLUP);
  pinMode(SWITCH_4_PIN, INPUT_PULLUP);

  for (int i = 0; i < 8; i++) {
    digitalWrite(LED_START + i, LOW);
    pinMode(LED_START + i, OUTPUT);
  }
  pinMode(STORE_PIN, INPUT_PULLUP);
  storeButton.attach(STORE_PIN);
  randomSeed(analogRead(0));
  comms.begin();
  writeLeds(0x00);
  newGame();
  comms.gameEnabled(true);
  comms.gameRunning(true);
}

void newGame() {
  completeCount = 0;
  comms.gameState(GAMESTATE_Unsolved);
  newTarget();
  refreshLeds();
}
void newTarget() {
  targetValue = random(0, 16);
}
void refreshLeds() {
  uint8_t leds = targetValue & 0x0F;
  for (int i = 0; i < completeCount; i++) {
    leds |= (0x01 << (i+4));
  }
  writeLeds(leds);
  
}
void writeLeds(uint8_t leds) {
  for (int i = 0; i < 8; i++) {
    if (leds & (0x01 << i)) {
      digitalWrite(LED_START + i, HIGH);
    } else {
      digitalWrite(LED_START + i, LOW);
    }
  }
}

void storePressed() {
  uint8_t currentValue = 0x00;
  for (int i = 0; i < 4; i++) {
    currentValue |= !digitalRead(SWITCH_1_PIN+i) << i;
  }
  if (currentValue == targetValue) {
    completeCount++;
    newTarget();
  } else {
    completeCount = 0;
    newTarget();
  }
  if (completeCount == 4) {
    comms.gameState(GAMESTATE_Solved);
    targetValue = 0x00; 
  }
  refreshLeds();
}
void enableGame() {
  refreshLeds();
}

void loop() {
  if (comms.gameRunning()) {
    if (comms.gameEnabled()) {
      if (comms.gameState() != GAMESTATE_Solved) {
        if (storeButton.update()) {
          if (!storeButton.read()) {
            storePressed();
          }
        }
      }
    } else {
      if (comms.gameState() != GAMESTATE_Solved) {
         if (lastDisplay + 500 < millis()) {
          lastDisplay = millis();
          flashOn = !flashOn;
          if (flashOn) {
            writeLeds(0xFF);
          } else {
            writeLeds(0x00);
          }
         }
      }
    }
  }
  comms.receiveMessage();

}
