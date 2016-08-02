#include <RobbieComms.h>

#define COMM_RX 10
#define COMM_TX 11
#define COMM_ENABLE 12

#define DEVICE_ID 0x02

#define SHIFTOUT_CLOCK 4
#define SHIFTOUT_DATA 2
#define SHIFTOUT_LATCH 3

#define RED_OUTPUT 5
#define GREEN_OUTPUT 6
#define YELLOW_OUTPUT 7
#define BLUE_OUTPUT 8
#define WHITE_OUTPUT 9

#define RED_INPUT A1
#define GREEN_INPUT A2
#define YELLOW_INPUT A3
#define BLUE_INPUT A4
#define WHITE_INPUT A5

#define RED_LED 0x01
#define GREEN_LED 0x02
#define YELLOW_LED 0x04
#define BLUE_LED 0x08
#define WHITE_LED 0x10

uint8_t wireSolution[5];
bool solved[5];

uint32_t lastDisplay = 0;
uint32_t lastCheck = 0;
uint8_t wireLoop = 0;

uint8_t ledMap[] = {RED_LED, GREEN_LED, YELLOW_LED, BLUE_LED, WHITE_LED };
uint8_t inputs[] = {RED_INPUT, GREEN_INPUT, YELLOW_INPUT, BLUE_INPUT, WHITE_INPUT };
uint8_t outputs[] = {RED_OUTPUT, GREEN_OUTPUT, YELLOW_OUTPUT, BLUE_OUTPUT, WHITE_OUTPUT };

RobbieComms comms(DEVICE_ID);

void setup() {
  Serial.begin(9600);
  comms.begin();
  comms.setCallBacks(newGame, 0, 0);
  
  pinMode(SHIFTOUT_DATA, OUTPUT);
  pinMode(SHIFTOUT_CLOCK, OUTPUT);
  pinMode(SHIFTOUT_LATCH, OUTPUT);   
  
  randomSeed(analogRead(0));
  delay(100);
  for (int i = 0 ; i < 5; i++) {
    pinMode(inputs[i], INPUT_PULLUP);
    pinMode(outputs[i], OUTPUT);
  }
  writeLeds(0x0000);
  newGame();
  comms.gameEnabled(true);
  comms.gameRunning(true);
  
}

void loop() {
  if (comms.gameEnabled() && comms.gameRunning()) {
    if (lastDisplay + 500 < millis()) {
      lastDisplay = millis();
      setLeds();
    }
    if (lastCheck + 100 < millis()) {
      lastCheck = millis();
      comms.gameState(checkSolution());
    }
  } else {
    if (lastDisplay + 1500 < millis()) {
      lastDisplay = millis();
      writeLeds(0x0000);
    }
  }
  comms.receiveMessage();
}

void setLeds() {
  if (comms.gameState() == GAMESTATE_Solved) {
    writeLeds(0x0000);
    return;
  }
  while (solved[wireLoop] && wireLoop != 5) {
    wireLoop++;
  }
  if (wireLoop == 5) {
    wireLoop = 0;
    writeLeds(0x0000);
    return;
  }
  uint16_t leds = 0x0000;
  leds |= ledMap[wireLoop];
  leds |= (ledMap[wireSolution[wireLoop]] << 8);
  writeLeds(leds);
  wireLoop++;
}

void writeLeds(uint16_t leds) {
  digitalWrite(SHIFTOUT_CLOCK, LOW );
  digitalWrite(SHIFTOUT_LATCH, LOW);
  
  shiftOut(SHIFTOUT_DATA, SHIFTOUT_CLOCK, MSBFIRST, leds >> 8);
  shiftOut(SHIFTOUT_DATA, SHIFTOUT_CLOCK, MSBFIRST, leds & 0xFF);
  digitalWrite(SHIFTOUT_CLOCK, HIGH);
  digitalWrite(SHIFTOUT_LATCH, HIGH);
    
}

bool checkSolution() {
  bool gameSolved = true;
  for (int i = 0; i < 5; i++) {
    digitalWrite(outputs[i], HIGH);  
    solved[i] = true;
  }
  for (int i = 0; i < 5; i++) {
    if (i != 0) {
      digitalWrite(outputs[i-1], HIGH);
    }
    digitalWrite(outputs[i], LOW);
    for (int y = 0; y < 5; y++) {
      if (!digitalRead(inputs[y])) {
        if (wireSolution[i] != y) {
          gameSolved = false;
          solved[i] = false;
          break;  
        }
      } else if (wireSolution[i] == y) {
        gameSolved = false;
        solved[i] = false;
        break;
      }
    }
  }
  return gameSolved;
}

void newGame() {
  uint8_t available[5];
  uint8_t slot;
  for (int i = 0; i < 5; i++) {
    available[i] = i;
  }
  for (int i = 0; i < 5; i++) {
      for (int y = 0; y < 5; y++) {
      }
      slot = random(0, 4-i);
      wireSolution[i] = available[slot];
      if (slot != 4-i) {
        available[slot] = available[4-i];
      } 
  }
}

