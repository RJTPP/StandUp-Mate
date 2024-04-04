/*
 * Copyright (c) 2024 Rajata Thamcharoensatit
 *
 * This software is licensed under the MIT License. Please see the LICENSE file for details.
 *
 * SPDX-License-Identifier: MIT
 *
 * For more information and contributions, please visit:
 * https://github.com/RJTPP/StandUp-Mate
 */


#include <WiFi.h>
#include "PubSubClient.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <HCSR04.h>

#include "hardwareConfig.h"
#include "standUpMate.h"


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Ultrasonic
UltraSonicDistanceSensor distanceSensor(US_TRIGGER_PIN, US_ECHO_PIN);

// ** Setup **
void setup() {
  // Debug
  Serial.begin(115200);

  // Set pin mode
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);

  digitalWrite(LED_GREEN_PIN, LOW);
  analogWrite(LED_YELLOW_PIN, 48);

  // Set SDA and SCL
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);

  // Check display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    // Do nothing
    for (;;)
      ;
  }

  // Setup screen
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  delay(1000);

  // Booting animation
  renderBootPage(1);
  delay(300);


  digitalWrite(LED_GREEN_PIN, HIGH);
  analogWrite(LED_YELLOW_PIN, 0);
  delay(1000);
}


// ** Loop **
void loop() {
  currentInternalTime = millis();

  // Switch
  switchStatus = digitalRead(SWITCH_PIN);

  // Ultrasonic
  if ((currentInternalTime - lastDistanceLoop >= 500)) {
    updateUltrasonic();
    showLedSittingStatus();
    lastDistanceLoop = millis();
  }

  // Timer
  updateTimer();
  int remainingSecond = timerDurationSec - sittingTime;

  // Stand up
  if (remainingSecond <= 0) {
    // If coming back
    if (goingOut && isSitting) {
      goingOut = false;
      currentPage = lastCurrentPage;
      sittingTime = 0;
      remainingSecond = timerDurationSec;
    }
    // Time to stand -> standing
    else if (!goingOut && !isSitting) {
      goingOut = true;
      currentPage = COMPLEMENT_PAGE;
    }
    // Time to stand -> still sit
    else if (!goingOut && isSitting) {
      if (currentPage != STAND_PAGE) lastCurrentPage = currentPage;
      currentPage = STAND_PAGE;
    }
  }

  // Timer 2
  int *sittingTimeList = secondToHour(sittingTime);
  int *remainingList = secondToHour(remainingSecond);

  // Preparing to rendering new frame
  display.clearDisplay();
  updateCurrentPage();

  showPagination = true;

  // Page rendering
  switch (currentPage) {
    case 1:
      renderRemainingTimePage(remainingList[0], remainingList[1], remainingList[2]);
      break;
    case 2:
      renderRemainingBarPage(remainingSecond, remainingList[0], remainingList[1]);
      break;
    case 3:
      display.clearDisplay();
      showPagination = false;
      break;
    case CALIBRATION_PAGE:
      showPagination = false;
      renderCalibrationPage();
      break;
    case STAND_PAGE:
      showPagination = false;
      renderStandPage();

      // Siren LED
      if (millis() - lastLedSiren > 100) {
        switch (ledNum) {
          case 1:
            digitalWrite(LED_GREEN_PIN, HIGH);
            analogWrite(LED_YELLOW_PIN, 0);
            digitalWrite(LED_RED_PIN, LOW);
            break;
          case 2:
            digitalWrite(LED_GREEN_PIN, LOW);
            analogWrite(LED_YELLOW_PIN, 255);
            digitalWrite(LED_RED_PIN, LOW);
            break;
          case 3:
            digitalWrite(LED_GREEN_PIN, LOW);
            analogWrite(LED_YELLOW_PIN, 0);
            digitalWrite(LED_RED_PIN, HIGH);
            break;
        }
        // Switch LED
        ledNum = (ledNum >= 3) ? 1 : ledNum + 1;
        lastLedSiren = millis();
      }
      break;
    case COMPLEMENT_PAGE:
      renderComplementPage();
      showPagination = false;
      break;

    default:
      printf("switch case error %d\n", currentPage);
  }

  if (showPagination) {
    renderPagination(maxPage, currentPage, radius, margin, false);
  }


  delay(16);

  // Render
  display.display();

  // int *x = secondToHour(sittingTime);

  // printf("%f, %f, %d, %d, %d, %d, %d %d, %d, %d\n", sittingDistance, currentDistance, isSitting, calibrating, ledNum, lastLedSiren, sittingTime, x[0], x[1], x[2]);
}


// ** Function **


// * Sitting *

void updateUltrasonic() {
  currentDistance = distanceSensor.measureDistanceCm();
  if ((currentDistance - sittingDistance >= 30.0) || currentDistance < 0) {
    isSitting = false;

  } else {
    isSitting = true;
  }
}

void updateTimer() {
  static uint32_t lastInternalTime = currentInternalTime;
  if (currentInternalTime - lastInternalTime >= 1000) {
    sittingTime++;
    // sittingTime += 500;
    lastInternalTime = currentInternalTime;
  }
}

void showLedSittingStatus() {
  if ((isSitting) & (currentPage != 7)) {
    digitalWrite(LED_GREEN_PIN, HIGH);
    analogWrite(LED_YELLOW_PIN, 0);
    digitalWrite(LED_RED_PIN, LOW);
  } else {
    digitalWrite(LED_GREEN_PIN, LOW);
    analogWrite(LED_YELLOW_PIN, 48);
    digitalWrite(LED_RED_PIN, LOW);
  }
}


// * Time *

int *secondToHour(int rawSecond) {
  static int time[3];

  int second = rawSecond % 60;
  int min = (rawSecond / 60) % 60;
  int hour = rawSecond / 3600;

  time[0] = hour;
  time[1] = min;
  time[2] = second;

  return time;
}


// * Togling *

// prevent long press
void updateToggleStatus() {
  if (digitalRead(SWITCH_PIN) == 0) {
    if (!isLongPress) {
      isLongPress = true;
      isToggling = !isToggling;
    }
  } else {
    isLongPress = false;
  }
  return;
}

bool checkToggling() {
  updateToggleStatus();
  if (isToggling) {
    isToggling = false;
    return true;
  }
  return false;
}

bool updateCurrentPage() {
  if (checkToggling()) {
    if (currentPage == CALIBRATION_PAGE) {
      calibrating = false;
      sittingTime = 0;
    } else if (currentPage == STAND_PAGE) {
      sittingTime = 0;
      currentPage = lastCurrentPage;
      return true;
    }

    currentPage = (currentPage >= maxPage) ? 1 : currentPage + 1;
    return true;
  }
  return false;
}


// * Element Renderers*

void renderOneLineText(char text[], int startY) {
  display.clearDisplay();
  display.setCursor(0, startY);
  display.println(text);
  display.display();
}

// New line when text is to long for one line
void renderMultiLineText(char text[], int startY, int gapY, bool renderNow) {
  // printf("%s", text);
  display.clearDisplay();

  int lenght = strlen(text);
  int remainingChar = lenght;
  int cursorYCoordinate = startY;
  char tempString[23] = "";

  // Cut to chunks
  for (int i = 0; i < lenght; i += maxCharPerLine) {
    display.setCursor(0, cursorYCoordinate);

    if (remainingChar <= maxCharPerLine) {
      strncpy(tempString, text + i, remainingChar);
      tempString[remainingChar] = '\0';
    } else {
      strncpy(tempString, text + i, maxCharPerLine);
      tempString[maxCharPerLine + 1] = '\0';
      remainingChar -= maxCharPerLine;
    }
    // printf("render: %s\n", tempString);
    display.println(tempString);
    cursorYCoordinate += gapY;
  }
  if (renderNow) {
    display.display();
  }

  return;
}

// Vertical scroll for vertical overflow text
void renderMultiLineTextScroll(char text[], int startY, int gapY, bool pagination) {
  // printf("%s", text);
  display.clearDisplay();

  int scrollNumber = 0;  // Number of top render line on screen
  int lenght = strlen(text);
  int maxTextLine = (lenght / maxCharPerLine) + 5;
  uint32_t lastLoopTime = millis();

  // Cut to chunks
  while (scrollNumber <= maxTextLine * textHeight) {
    // Realtime switch check
    updateToggleStatus();
    if (updateCurrentPage()) {
      return;
    }

    // Render delays
    if (millis() - lastLoopTime <= 60) {
      continue;
    }

    renderMultiLineText(text, startY, gapY, false);

    if (pagination && (scrollNumber >= ((maxTextLine - 3) * textHeight))) {
      renderPagination(maxPage, currentPage, radius, margin, false);
    }

    renderScrollBar(maxTextLine * textHeight, scrollNumber);

    display.display();

    startY--;
    scrollNumber++;
    lastLoopTime = millis();
  }

  return;
}

void renderPagination(int maxPage, int currentPage, int radius, int margin, bool renderNow) {
  int diameter = radius * 2;
  int usedY = ((diameter + margin) * maxPage) - margin;  // equal to (diameter * maxPage) + (margin * (maxPage - 1))แ
  int startX = (SCREEN_WIDTH - usedY) / 2;
  int startY = SCREEN_HEIGHT - diameter;

  for (int i = 0; i < maxPage; i++) {
    const int offsetX = (i == 0) ? 0 : ((i * (diameter + margin)));

    if (i + 1 == currentPage) {
      display.fillCircle((startX + offsetX), startY, (radius + 1), WHITE);
      continue;
    }

    display.drawCircle((startX + offsetX), startY, radius, WHITE);
  }

  if (renderNow) {
    display.display();
  }

  return;
}

void renderScrollBar(int maxStep, int currentStep) {
  int barHeight = 12;
  int barY = (currentStep / (float)maxStep) * (SCREEN_HEIGHT - barHeight);

  display.fillRoundRect(SCREEN_WIDTH - 4, barY, 4, barHeight, 2, WHITE);
}


// * // Page Renderers *

void renderBootPage(int loadingDelay) {
  const int startX = 12;

  display.clearDisplay();

  display.setCursor(31, 9);
  display.println("Starting up");
  display.drawRoundRect(startX, 30, 101, 12, 5, WHITE);
  display.display();
  delay(500);

  for (int i = 0; i <= 100; i += 2) {
    display.clearDisplay();

    char strI[10] = { 0 };
    sprintf(strI, "%d%%", i);

    // text
    display.setCursor(31, 9);
    display.println("Starting up");
    display.setCursor((i + startX - 12), 50);
    display.println(strI);

    // Progress bar
    display.drawRoundRect(startX, 30, 101, 12, 5, WHITE);
    display.fillRoundRect(startX, 30, (i), 12, 5, WHITE);

    // Render
    display.display();
    delay(loadingDelay);
  }

  return;
}

void renderCalibrationPage() {
  calibrating = true;
  sittingTime = 0;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Setup distance sensor");
  display.drawRoundRect(12, 21, 101, 12, 5, WHITE);

  sittingDistance = currentDistance;


  if ((0.0 < sittingDistance) && (sittingDistance < 60.0)) {
    display.setCursor(52, 41);
    display.print("Good");
    display.setCursor(28, 57);
    display.println("Press button");
    // display.fillRoundRect(12, 30, 100, 12, 5, WHITE);
  } else if ((0.0 < sittingDistance) && (sittingDistance < 100.0)) {
    display.setCursor(57, 41);
    display.println("Ok");
    // display.fillRoundRect(12, 66, 100, 12, 5, WHITE);
  } else {
    display.setCursor(25, 41);
    display.println("Not detected!");
    // display.fillRoundRect(12, 33, 100, 12, 5, WHITE);
  }

  int w = (200 - (float)sittingDistance) / 2;
  if ((w < 0) || (sittingDistance == -1)) {
    w = 0;
  }
  display.fillRoundRect(12, 21, w, 12, 5, WHITE);

  display.display();
}

void renderRemainingTimePage(int hour, int min, int second) {
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.printf("%dh %dm %ds", hour, min, second);

  display.setTextSize(1);
}

void renderRemainingBarPage(int RawremainingSec, int Rawremaininghour, int Rawremainingmin) {
  int outterBarwidth = SCREEN_WIDTH - 1;
  int innerBarWidth = (RawremainingSec / (float)timerDurationSec) * outterBarwidth;

  display.drawRoundRect(0, 30, outterBarwidth, 12, 5, WHITE);
  display.fillRoundRect(0, 30, innerBarWidth, 12, 5, WHITE);

  int xCursor = (innerBarWidth - 50);
  xCursor = (xCursor > 0) ? xCursor : 0;

  display.setCursor(xCursor, 15);
  display.printf("%dh %dm", Rawremaininghour, Rawremainingmin);
}

void renderStandPage() {
  display.setTextSize(2);
  display.setCursor(10, 26);
  display.print("Stand up!");

  display.setTextSize(1);
}

void renderComplementPage() {
  display.setTextSize(2);
  display.setCursor(10, 26);
  display.print("Good job!");

  display.setTextSize(1);
}
