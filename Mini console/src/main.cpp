
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// ========================================
// MINI PACKET OS - SUPER LITE
// Arduino Nano ATmega328P
// OLED SSD1306 128x64
// ========================================

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C

#define BTN_UP    2
#define BTN_DOWN  3
#define BTN_OK    4
#define BTN_BACK  5

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

// ========================================
// BUTTON SYSTEM
// ========================================

const byte buttonPins[4] = {
  BTN_UP, BTN_DOWN, BTN_OK, BTN_BACK
};

bool stableState[4] = {HIGH, HIGH, HIGH, HIGH};
bool lastReading[4] = {HIGH, HIGH, HIGH, HIGH};
bool buttonPressed[4] = {false, false, false, false};
bool buttonHeld[4] = {false, false, false, false};

unsigned long buttonTimer[4] = {0};

void updateButtons() {
  for (byte i = 0; i < 4; i++) {
    buttonPressed[i] = false;

    bool reading = digitalRead(buttonPins[i]);

    if (reading != lastReading[i]) {
      lastReading[i] = reading;
      buttonTimer[i] = millis();
    }

    if (millis() - buttonTimer[i] >= 25) {
      if (reading != stableState[i]) {
        stableState[i] = reading;

        if (stableState[i] == LOW) {
          buttonPressed[i] = true;
        }
      }
    }

    buttonHeld[i] = (stableState[i] == LOW);
  }
}

bool UP()   { return buttonPressed[0]; }
bool DOWN() { return buttonPressed[1]; }
bool OK()   { return buttonPressed[2]; }
bool BACK() { return buttonPressed[3]; }

// ========================================
// OS STATE
// ========================================

enum Screen {
  MAIN_MENU,
  APP_MENU,
  GAME,
  GAME_OVER,
  APP,
  INFO,
  ABOUT
};

Screen screen = MAIN_MENU;

byte menuIndex = 0;
byte currentApp = 0;

const char* mainItems[] = {
  "Applications",
  "Game",
  "System Info",
  "About"
};

const char* appItems[] = {
  "Calculator",
  "Stopwatch"
};

// ========================================
// CALCULATOR VARIABLES
// ========================================

int calcA = 0;
int calcB = 0;
int calcResult = 0;

byte calcStep = 0;
byte calcOperation = 0;

// ========================================
// STOPWATCH VARIABLES
// ========================================

unsigned long stopwatchStart = 0;
unsigned long stopwatchElapsed = 0;
unsigned long appTimer = 0;

bool stopwatchRunning = false;

// ========================================
// PONG VARIABLES
// ========================================

int playerY = 25;
int cpuY = 25;

float ballX = 64;
float ballY = 32;

float ballVX = 2.0;
float ballVY = 1.2;

byte playerScore = 0;
byte cpuScore = 0;

unsigned long pongTimer = 0;
unsigned long pongDrawTimer = 0;

const byte PADDLE_HEIGHT = 15;

// ========================================
// FUNCTION DECLARATIONS
// ========================================

void showMainMenu();
void showAppMenu();
void showInfo();
void showAbout();
void handleMenu();

void startPong();
void resetBall();
void drawPong();
void updatePong();
void showGameOver();

void startApp(byte id);
void drawCalculator();
void handleCalculator();
void drawStopwatch();
void handleStopwatch();

// ========================================
// DISPLAY HELPERS
// ========================================

void clearScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
}

void drawHeader(const char* title) {
  display.setCursor(0, 0);
  display.print(title);
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
}

// ========================================
// MENU DISPLAY
// ========================================

void drawMenu(
  const char* title,
  const char** items,
  byte count
) {
  clearScreen();
  drawHeader(title);

  for (byte i = 0; i < count; i++) {
    int y = 15 + i * 12;

    if (i == menuIndex) {
      display.fillRect(0, y - 2, 128, 11, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }

    display.setCursor(4, y);
    display.print(items[i]);
  }

  display.setTextColor(SSD1306_WHITE);
  display.display();
}

// ========================================
// MAIN MENU
// ========================================

void showMainMenu() {
  screen = MAIN_MENU;
  menuIndex = 0;

  drawMenu("MINI PACKET OS", mainItems, 4);
}

void showAppMenu() {
  screen = APP_MENU;
  menuIndex = 0;

  drawMenu("APPLICATIONS", appItems, 2);
}

// ========================================
// SYSTEM INFO
// ========================================

void showInfo() {
  screen = INFO;

  clearScreen();
  drawHeader("SYSTEM INFO");

  display.setCursor(0, 15);
  display.println("MINI PACKET OS");
  display.println("Arduino Nano");
  display.println("ATmega328P");
  display.println("OLED: 128x64");
  display.println("RAM: 2 KB");

  display.setCursor(0, 56);
  display.print("BACK: MENU");

  display.display();
}

// ========================================
// ABOUT
// ========================================

void showAbout() {
  screen = ABOUT;

  clearScreen();
  drawHeader("ABOUT");

  display.setCursor(0, 18);
  display.println("MINI PACKET OS");
  display.println("SUPER LITE");
  display.println("NANO EDITION");
  display.println("Made with Arduino");

  display.setCursor(0, 56);
  display.print("BACK: MENU");

  display.display();
}

// ========================================
// MENU CONTROL
// ========================================

void handleMenu() {
  byte count;
  const char** items;
  const char* title;

  if (screen == MAIN_MENU) {
    count = 4;
    items = mainItems;
    title = "MINI PACKET OS";
  } else {
    count = 2;
    items = appItems;
    title = "APPLICATIONS";
  }

  bool changed = false;

  if (UP()) {
    menuIndex = (menuIndex + count - 1) % count;
    changed = true;
  }

  if (DOWN()) {
    menuIndex = (menuIndex + 1) % count;
    changed = true;
  }

  if (BACK()) {
    if (screen == APP_MENU) {
      showMainMenu();
    }
    return;
  }

  if (changed) {
    drawMenu(title, items, count);
  }

  if (OK()) {
    if (screen == MAIN_MENU) {
      if (menuIndex == 0) {
        showAppMenu();
      } else if (menuIndex == 1) {
        startPong();
      } else if (menuIndex == 2) {
        showInfo();
      } else {
        showAbout();
      }
    } else {
      startApp(menuIndex);
    }
  }
}

// ========================================
// PONG RESET
// ========================================

void resetBall() {
  ballX = 64;
  ballY = 32;

  ballVX = random(0, 2) ? 2.0 : -2.0;
  ballVY = random(0, 2) ? 1.2 : -1.2;
}

// ========================================
// START PONG
// ========================================

void startPong() {
  screen = GAME;

  playerY = 25;
  cpuY = 25;

  playerScore = 0;
  cpuScore = 0;

  resetBall();

  pongTimer = millis();
  pongDrawTimer = millis();

  drawPong();
}

// ========================================
// DRAW PONG
// ========================================

void drawPong() {
  clearScreen();
  drawHeader("PONG");

  display.setCursor(42, 0);
  display.print(playerScore);

  display.setCursor(85, 0);
  display.print(cpuScore);

  for (byte y = 14; y < 64; y += 8) {
    display.drawFastVLine(64, y, 4, SSD1306_WHITE);
  }

  display.fillRect(4, playerY, 3, PADDLE_HEIGHT, SSD1306_WHITE);
  display.fillRect(121, cpuY, 3, PADDLE_HEIGHT, SSD1306_WHITE);

  display.fillRect((int)ballX, (int)ballY, 3, 3, SSD1306_WHITE);

  display.display();
}

// ========================================
// UPDATE PONG
// ========================================

void updatePong() {
  if (millis() - pongTimer < 25) return;

  pongTimer = millis();

  // Player
  if (buttonHeld[0]) playerY -= 3;
  if (buttonHeld[1]) playerY += 3;

  playerY = constrain(playerY, 12, 49);

  // CPU
  int cpuCenter = cpuY + 7;

  if (ballY > cpuCenter + 2) cpuY += 2;
  if (ballY < cpuCenter - 2) cpuY -= 2;

  cpuY = constrain(cpuY, 12, 49);

  // Ball
  ballX += ballVX;
  ballY += ballVY;

  if (ballY <= 12) {
    ballY = 12;
    ballVY = -ballVY;
  }

  if (ballY >= 61) {
    ballY = 61;
    ballVY = -ballVY;
  }

  // Player collision
  if (
    ballVX < 0 &&
    ballX <= 7 &&
    ballX >= 4 &&
    ballY + 3 >= playerY &&
    ballY <= playerY + PADDLE_HEIGHT
  ) {
    ballX = 7;
    ballVX = -ballVX;
  }

  // CPU collision
  if (
    ballVX > 0 &&
    ballX + 3 >= 121 &&
    ballX <= 124 &&
    ballY + 3 >= cpuY &&
    ballY <= cpuY + PADDLE_HEIGHT
  ) {
    ballX = 118;
    ballVX = -ballVX;
  }

  // Player score
  if (ballX > 127) {
    playerScore++;
    resetBall();
  }

  // CPU score
  if (ballX < 0) {
    cpuScore++;
    resetBall();
  }

  if (playerScore >= 5 || cpuScore >= 5) {
    showGameOver();
  }
}

// ========================================
// GAME OVER
// ========================================

void showGameOver() {
  screen = GAME_OVER;

  clearScreen();
  drawHeader("GAME OVER");

  display.setCursor(25, 22);
  display.print("YOU: ");
  display.println(playerScore);

  display.setCursor(25, 32);
  display.print("CPU: ");
  display.println(cpuScore);

  display.setCursor(5, 49);
  display.print("OK: RESTART");

  display.setCursor(5, 58);
  display.print("BACK: MENU");

  display.display();
}

// ========================================
// START APPLICATION
// ========================================

void startApp(byte id) {
  currentApp = id;
  screen = APP;

  if (currentApp == 0) {
    calcA = 0;
    calcB = 0;
    calcResult = 0;
    calcStep = 0;
    calcOperation = 0;

    drawCalculator();
  } else {
    stopwatchElapsed = 0;
    stopwatchRunning = false;
    stopwatchStart = millis();
    appTimer = millis();

    drawStopwatch();
  }
}

// ========================================
// CALCULATOR DISPLAY
// ========================================

void drawCalculator() {
  clearScreen();
  drawHeader("CALCULATOR");

  display.setCursor(0, 15);

  if (calcStep == 0) {
    display.println("SET NUMBER A");
    display.print("A = ");
    display.println(calcA);
  } else if (calcStep == 1) {
    display.println("SELECT OPERATOR");
    display.println("UP: +");
    display.println("DOWN: -");

    display.print("OP: ");
    display.println(calcOperation == 0 ? "+" : "-");
  } else if (calcStep == 2) {
    display.println("SET NUMBER B");
    display.print("B = ");
    display.println(calcB);
  } else {
    display.println("RESULT");
    display.setTextSize(2);
    display.println(calcResult);
    display.setTextSize(1);
    display.println("OK: AGAIN");
  }

  display.setCursor(0, 56);
  display.print("BACK: EXIT");

  display.display();
}

// ========================================
// CALCULATOR CONTROL
// ========================================

void handleCalculator() {
  if (BACK()) {
    showAppMenu();
    return;
  }

  bool changed = false;

  if (calcStep == 0) {
    if (UP()) {
      calcA++;
      changed = true;
    }

    if (DOWN()) {
      calcA--;
      changed = true;
    }

    calcA = constrain(calcA, -99, 99);

    if (OK()) {
      calcStep = 1;
      changed = true;
    }
  } else if (calcStep == 1) {
    if (UP()) {
      calcOperation = 0;
      changed = true;
    }

    if (DOWN()) {
      calcOperation = 1;
      changed = true;
    }

    if (OK()) {
      calcStep = 2;
      changed = true;
    }
  } else if (calcStep == 2) {
    if (UP()) {
      calcB++;
      changed = true;
    }

    if (DOWN()) {
      calcB--;
      changed = true;
    }

    calcB = constrain(calcB, -99, 99);

    if (OK()) {
      if (calcOperation == 0) {
        calcResult = calcA + calcB;
      } else {
        calcResult = calcA - calcB;
      }

      calcStep = 3;
      changed = true;
    }
  } else {
    if (OK()) {
      calcA = 0;
      calcB = 0;
      calcResult = 0;
      calcStep = 0;
      changed = true;
    }
  }

  if (changed) {
    drawCalculator();
  }
}

// ========================================
// STOPWATCH DISPLAY
// ========================================

void drawStopwatch() {
  clearScreen();
  drawHeader("STOPWATCH");

  unsigned long elapsed = stopwatchElapsed;

  if (stopwatchRunning) {
    elapsed += millis() - stopwatchStart;
  }

  unsigned long totalSeconds = elapsed / 1000;
  unsigned int minutes = totalSeconds / 60;
  unsigned int seconds = totalSeconds % 60;

  display.setTextSize(2);
  display.setCursor(15, 23);

  if (minutes < 10) display.print("0");
  display.print(minutes);
  display.print(":");

  if (seconds < 10) display.print("0");
  display.print(seconds);

  display.setTextSize(1);

  display.setCursor(0, 49);
  display.print("OK: START/STOP");

  display.setCursor(0, 58);
  display.print("UP: RESET");

  display.display();
}

// ========================================
// STOPWATCH CONTROL
// ========================================

void handleStopwatch() {
  if (BACK()) {
    showAppMenu();
    return;
  }

  if (UP()) {
    stopwatchElapsed = 0;

    if (stopwatchRunning) {
      stopwatchStart = millis();
    }

    drawStopwatch();
  }

  if (OK()) {
    if (stopwatchRunning) {
      stopwatchElapsed += millis() - stopwatchStart;
      stopwatchRunning = false;
    } else {
      stopwatchStart = millis();
      stopwatchRunning = true;
    }

    drawStopwatch();
  }

  if (stopwatchRunning && millis() - appTimer >= 250) {
    appTimer = millis();
    drawStopwatch();
  }
}

// ========================================
// SETUP
// ========================================

void setup() {
  for (byte i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  Wire.begin();
  Wire.setClock(100000);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    while (true) {
    }
  }

  randomSeed(analogRead(A0));

  clearScreen();

  display.setTextSize(2);
  display.setCursor(15, 18);
  display.println("MINI OS");

  display.setTextSize(1);
  display.setCursor(30, 48);
  display.println("SUPER LITE");

  display.display();

  delay(700);

  showMainMenu();
}

// ========================================
// MAIN LOOP
// ========================================

void loop() {
  updateButtons();

  switch (screen) {

    case MAIN_MENU:
    case APP_MENU:
      handleMenu();
      break;

    case GAME:
      if (BACK()) {
        showMainMenu();
        break;
      }

      updatePong();

      if (
        screen == GAME &&
        millis() - pongDrawTimer >= 35
      ) {
        pongDrawTimer = millis();
        drawPong();
      }
      break;

    case GAME_OVER:
      if (OK()) {
        startPong();
      }

      if (BACK()) {
        showMainMenu();
      }
      break;

    case APP:
      if (currentApp == 0) {
        handleCalculator();
      } else {
        handleStopwatch();
      }
      break;

    case INFO:
    case ABOUT:
      if (BACK() || OK()) {
        showMainMenu();
      }
      break;
  }
}
