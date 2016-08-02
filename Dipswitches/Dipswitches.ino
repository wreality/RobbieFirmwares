#include <Adafruit_LEDBackpack.h>
#include <RobbieComms.h>

#define COMM_RX 8
#define COMM_TX 9
#define COMM_ENABLE 12

#define DEVICE_ID 0x01

#define SHIFT_CLOCK 4
#define SHIFT_DATA 5
#define SHIFT_LATCH 3

uint16_t targetValue;
uint16_t dipValue;

uint32_t lastDisplay = 0;
uint32_t lastCycle = 0;

Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

RobbieComms comms(DEVICE_ID);
void setup() {
  Serial.begin(9600);
  comms.begin();
  comms.setCallBacks(newGame, enableGame, disableGame);
  
  pinMode(SHIFT_DATA, INPUT_PULLUP);
  pinMode(SHIFT_CLOCK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);   
  
  randomSeed(analogRead(0));
  alpha4.begin(0x70);
  newGame();
  comms.gameRunning(true);
  comms.gameEnabled(true);
  
}

void loop() {
  if (comms.gameRunning()) {
    if ((!comms.gameEnabled()) && (lastCycle + 500 < millis())) {
        lastCycle = millis();
        cycleGame();
    } else if (comms.gameEnabled() && (lastDisplay + 100 < millis())) {
        lastDisplay = millis();
        checkSolution();
        writeDisplay();
    }
  } else {
    blankDisplay();
  }
  if (comms.receiveMessage()) {
  }
}

bool checkSolution() {
  //Toggle Shift latch
  digitalWrite(SHIFT_LATCH, HIGH);
  delayMicroseconds(50);
  digitalWrite(SHIFT_CLOCK, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);

  //Start the clock high to avoid being off by one;
  
  uint16_t high = shiftIn(SHIFT_DATA, SHIFT_CLOCK, LSBFIRST) << 8;
  uint16_t low = shiftIn(SHIFT_DATA, SHIFT_CLOCK, LSBFIRST);

  dipValue = high | low;
  if (targetValue == dipValue) {
    comms.gameState(GAMESTATE_Solved);
  } else {
    comms.gameState(GAMESTATE_Unsolved);
  }
}

void cycleGame() {
  targetValue = random(0xFFFF);
  writeDisplay();
}

void writeDisplay() {
  uint8_t nibble = 0;
  for (int i = 0; i <= 3; i++) {
    nibble = (targetValue >> (4 * i)) & 0x0F;
    if ((nibble != ((dipValue >> (4*i)) & 0x0F)) || !comms.gameEnabled()) { 
      if (nibble < 0xA) {
        alpha4.writeDigitAscii((i-3)*-1, (char)('0'+nibble), false);
      } else {
        alpha4.writeDigitAscii((i-3)*-1, (char)('A'+(nibble- 0xA)), false);
      }
    } else {
      alpha4.writeDigitAscii((i-3)*-1, ' ', true);
    }
  }
  alpha4.writeDisplay();
}

void blankDisplay() {
  for (int i = 0; i <= 3; i++) {
    alpha4.writeDigitAscii(i, ' ', false);
  }
  alpha4.writeDisplay();
}

void enableGame() {
  
}
void disableGame() {
  
}
void newGame() {
  cycleGame();
}

