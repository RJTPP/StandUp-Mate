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


#include "esp32-hal.h"
#include <stdint.h>

enum specialPage {
  CALIBRATION_PAGE = 4,
  STAND_PAGE = 5,
  COMPLEMENT_PAGE = 6
};

// ** Variable **

// Time
uint32_t currentInternalTime = millis();

// Max per line: 21
// Max line: 7
// Max char: 147
const int textWidth = 6;
const int textHeight = 8;
const int maxCharPerLine = ((int)(SCREEN_WIDTH / textWidth)) - 1;  //21 - 1

// Switch
int switchStatus = 1;
bool isToggling = false;
bool isLongPress = false;

// Pagination
const int radius = 2;
const int margin = 3;
const int maxPage = 3;
int currentPage = CALIBRATION_PAGE;
int lastCurrentPage = 1;
bool showPagination = false;

// Interval
uint32_t lastDebugLoop = millis();
uint32_t lastDistanceLoop = millis();

// time
int sittingTime = 1;
int timerDurationMin = 45;
int timerDurationSec = timerDurationMin * 60;
// int timerDurationSec = 10;

// sitting data
float sittingDistance = 0.0;
float currentDistance = 0.0;
bool isSitting = true;
bool goingOut = false;
bool calibrating = true;
int ledNum = 1;
uint32_t lastLedSiren = millis();


// ** Function Delaration **


// Sitting
void updateUltrasonic();
void showLedSittingStatus();
void updateTimer();

// Time
int *secondToHour(int rawSecond);

// Togling
void updateToggleStatus();
bool checkToggling();
bool updateCurrentPage();

// Element Renderers
void renderOneLineText(char text[], int startY);
void renderMultiLineText(char text[], int startY, int gapY, bool rendetNow);
void renderMultiLineTextScroll(char text[], int startY, int gapY, bool pagination);
void renderPagination(int maxPage, int currentPage, int radius, int margin, bool renderNow);
void renderScrollBar(int maxStep, int currentStep);

// Page Renderers
void renderCalibrationPage(int loadingDelay);
void renderSetupPage();
void renderRemainingTimePage(int hour, int min, int second);
void renderRemainingBarPage(int RawremainingSec, int Rawremaininghour, int Rawremainingmin);
void renderStandPage();
void renderComplementPage();
