#include <Bounce2.h>
#include <RobbieComms.h>

#define COMM_RX 8
#define COMM_TX 9
#define COMM_ENABLE 10

#define DEVICE_ID 0x00

RobbieComms comms(DEVICE_ID);

#define CP_PIN A4
#define CP_GROUND A6 

#define FIXED_PIN 7
#define METER_PIN 3
#define LAMP_PIN 2

#define RESET_PIN A0
#define RESET_GROUND A2 

Bounce cpPin  = Bounce();
Bounce resetPin = Bounce();

uint8_t numModules = 0;
uint8_t moduleIds[6];

uint32_t lastTime = 0;
uint32_t buttonPress = 0;

uint16_t gameTimer;
int8_t gameTimerIncrement;

void setup() {

  pinMode(CP_PIN, INPUT_PULLUP);
  pinMode(CP_GROUND, OUTPUT);
  digitalWrite(CP_GROUND, LOW);
  
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(RESET_GROUND, OUTPUT);
  digitalWrite(RESET_GROUND, LOW);
  
  pinMode(METER_PIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  pinMode(FIXED_PIN, OUTPUT);

  digitalWrite(FIXED_PIN, HIGH);
  lampOn(false);
  
  cpPin.attach(CP_PIN);
  cpPin.interval(100);

  resetPin.attach(RESET_PIN);
  resetPin.interval(5);
  
  Serial.begin(9600);
  comms.begin();

  //Delay long enough for the slave modules to get up and running.
  analogWrite(METER_PIN, 255);
  
  delay(1000);
  
  comms.masterAnnounce(true);
  setupGames();
  
  analogWrite(METER_PIN, 0);
  
}

void loop() {
  bool newGame = false;
  if (resetPin.update()) {
    if (!resetPin.read()) {
      comms.gameRunning(true);
      comms.masterAnnounce(true);
      newGame = true;
      digitalWrite(FIXED_PIN, LOW);
      lampOn(false);
      gameTimer = 255;
      buttonPress = millis();
    } else {
      buttonPress = 0;
    }
  } else if (buttonPress != 0 && buttonPress + 5000 < millis()) {
    buttonPress = 0;
    lampOn(true);
    delay(2000);
    lampOn(false);
    setupGames();
  }
  if (cpPin.update() || newGame) {
    int cpState = !cpPin.read();
    if (cpState != comms.gameEnabled()) {
      comms.gameEnabled(cpState);
      comms.masterAnnounce(false);
    }
    if (comms.gameRunning()) {
      if (cpState) {
        lampOn(false);
        gameTimerIncrement = -3;
      } else {
        if (allModulesComplete()) {
          gameTimerIncrement = 25;
          lampOn(false);
          digitalWrite(FIXED_PIN, HIGH);
          comms.gameRunning(false);
          comms.masterAnnounce(false);
        } else {
          gameTimerIncrement = -15;
          lampOn(true);
        }
      }
    }
  }
  if (lastTime + 3000 < millis()) {
    lastTime = millis(); 
    if (gameTimerIncrement > 0) {
      if ((255 - gameTimerIncrement) < gameTimer) {
        gameTimer = 255;
      } else {
        gameTimer += gameTimerIncrement;
      }
    } else if (gameTimerIncrement < 0) {
      if ((-1*gameTimerIncrement > gameTimer)) {
        gameTimer = 0;
      } else {
        gameTimer += gameTimerIncrement;
      }
    }
      
    analogWrite(METER_PIN, gameTimer);
    if (gameTimer == 0 && comms.gameRunning()) {
        lampOn(true);
        comms.gameRunning(false);
        comms.masterAnnounce(false);

        delay(5000);
        lampOn(false);
    }
  }
}

bool allModulesComplete() {
  bool returnValue = true;
  for (int i = 0; i < numModules; i++) {
    if (comms.masterRequestStatus(moduleIds[i])) {
      if (comms.incomingMessage.game_state == GAMESTATE_Unsolved) {
        returnValue = false;
      }
    }
  }
  return returnValue;
}

void lampOn(bool state) {
  if (state) {
    digitalWrite(LAMP_PIN, HIGH);
  } else {
    digitalWrite(LAMP_PIN, LOW);
  }
}
void setupGames() {
  numModules = 0;
  for (int i = 1; i < 9; i++) {
    if (comms.masterRequestStatus(i)) {
      moduleIds[numModules] = i;
      numModules++;
    }
  }
}

