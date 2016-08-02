#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <RobbieComms.h>
#include "background.h"
#include "bottom_1.h"
#include "bottom_2.h"
#include "right_1.h"
#include "right_2.h"
#include "top_1.h"
#include "top_2.h"
#include "left_1.h"
#include "left_2.h"


#define OLED_RESET 6
Adafruit_SSD1306 display(OLED_RESET);

#define COMM_RX 10
#define COMM_TX 11
#define COMM_ENABLE 12

#define DEVICE_ID 0x04

#define SHIFTOUT_CLOCK 7
#define SHIFTOUT_DATA 8
#define SHIFTOUT_LATCH 9



#define LEFT_1_SWITCH 2
#define LEFT_2_SWITCH 3
#define RIGHT_1_SWITCH 4
#define RIGHT_2_SWITCH 5
#define TOP_1_SWITCH A0
#define TOP_2_SWITCH A1
#define BOTTOM_1_SWITCH A2
#define BOTTOM_2_SWITCH A3

#define ICON_X 0
#define ICON_Y 0
#define ICON_SIZE 48

#define LEFT 0
#define RIGHT 1
#define TOP 2
#define BOTTOM 3

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect");
#endif

uint8_t solution[4];
uint8_t ledMap[] = { LEFT_1_SWITCH, LEFT_2_SWITCH, RIGHT_1_SWITCH, RIGHT_2_SWITCH, TOP_1_SWITCH, TOP_2_SWITCH, BOTTOM_1_SWITCH, BOTTOM_2_SWITCH};
uint32_t lastDisplay = 0;
uint32_t lastCheck = 0;

RobbieComms comms(COMM_RX, COMM_TX, COMM_ENABLE, DEVICE_ID);

void setup() {
  Serial.begin(9600);
  comms.begin();
  comms.setCallBacks(newGame, enableGame, 0);
  comms.begin();
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x30);
  display.clearDisplay();
  display.display();
  
  pinMode(SHIFTOUT_DATA, OUTPUT);
  pinMode(SHIFTOUT_CLOCK, OUTPUT);
  pinMode(SHIFTOUT_LATCH, OUTPUT);   
  for (int i = 0 ; i < 7; i++) {
    pinMode(ledMap[i], INPUT_PULLUP);
  }
  
  randomSeed(analogRead(0));
  
  writeLeds(0x00);
  newGame();
  comms.gameEnabled(true);
  comms.gameRunning(true);
}

void loop() {
  if (comms.gameEnabled() && comms.gameRunning()) {
    if (lastCheck + 100 < millis()) {
      lastCheck = millis();
      setLeds();
      comms.gameState(checkSolution());
    }
  } else {
    if (lastDisplay + 1500 < millis()) {
      lastDisplay = millis();
      writeLeds(0x00);
      display.clearDisplay();
      display.display();
    }
  }
  comms.receiveMessage();
}

void setLeds() {
  uint8_t leds = 0x00;
  for (int i = 0; i < 8; i++) {
    if (!digitalRead(ledMap[i])) {
      leds |= (0x1 << i);
    }
  }
  writeLeds(leds);
}

void writeLeds(uint8_t leds) {
  digitalWrite(SHIFTOUT_CLOCK, LOW );
  digitalWrite(SHIFTOUT_LATCH, LOW);
  
  shiftOut(SHIFTOUT_DATA, SHIFTOUT_CLOCK, MSBFIRST, leds);
  digitalWrite(SHIFTOUT_CLOCK, HIGH);
  digitalWrite(SHIFTOUT_LATCH, HIGH);
    
}
uint8_t getPair(uint8_t pin1, uint8_t pin2) {
  uint8_t returnValue = 0x00;
  if (!digitalRead(pin1)) {
    returnValue |= pin1;
  }
  if (!digitalRead(pin2)) {
    returnValue != pin2;
  }
  return returnValue;
}

bool checkSolution() {
  uint8_t pair;
  for (int i = 0; i < 4; i++) {
    pair = getPair(ledMap[i*2], ledMap[i*2+1]);
    if (solution[0]) {
      if (pair != ledMap[i*2]) {
        return false;
      }
    } else {
      if (pair != ledMap[i*2+1]) {
        return false;
      }
    }
  }
 
  return true;
}
void writeDisplay() {
  display.clearDisplay();
  display.drawBitmap(ICON_X, ICON_Y, background, ICON_SIZE, ICON_SIZE, WHITE);
  if (solution[TOP]) {
    display.drawBitmap(ICON_X, ICON_Y, Top_1, ICON_SIZE, ICON_SIZE, WHITE);
  } else {
    display.drawBitmap(ICON_X, ICON_Y, Top_2, ICON_SIZE, ICON_SIZE, WHITE);
  }
  if (solution[BOTTOM]) {
    display.drawBitmap(ICON_X, ICON_Y, Bottom_1, ICON_SIZE, ICON_SIZE, WHITE);
  } else {
    display.drawBitmap(ICON_X, ICON_Y, Bottom_2, ICON_SIZE, ICON_SIZE, WHITE);
  }
  if (solution[LEFT]) {
    display.drawBitmap(ICON_X, ICON_Y, Left_1, ICON_SIZE, ICON_SIZE, WHITE);
  } else {
    display.drawBitmap(ICON_X, ICON_Y, Left_2, ICON_SIZE, ICON_SIZE, WHITE);
  }
  if (solution[RIGHT]) {
    display.drawBitmap(ICON_X, ICON_Y, Right_1, ICON_SIZE, ICON_SIZE, WHITE);
  } else {
    display.drawBitmap(ICON_X, ICON_Y, Right_2, ICON_SIZE, ICON_SIZE, WHITE);
  }
  display.display();
}
void newGame() {
  for (int i = 0; i < 5; i++) {
    solution[i] = random(0,1);
  }
  writeDisplay();
}

void enableGame() {
  writeDisplay();
  setLeds();
}


