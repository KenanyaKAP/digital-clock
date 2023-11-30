#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <RTClib.h>
#include <Bonezegei_DHT11.h>
#include <string.h>
#include "tinyFont.h"
#include "bigFont.h"

// ==================== Define Object and Constant ====================
// Display MAX7219
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define DISPLAY_CS_PIN 5
MD_Parola display = MD_Parola(HARDWARE_TYPE, DISPLAY_CS_PIN, MAX_DEVICES);

// RTC
RTC_DS1307 rtc;

// DHT
#define DHT_PIN 14
Bonezegei_DHT11 dht(DHT_PIN);

// Button
#define MODE_BTN_PIN 2
#define UP_BTN_PIN 0
#define DOWN_BTN_PIN 4

// Buzzer
#define BUZZER_PIN 33

// LDR
#define LDR_PIN 27

// ClockMode Enum
enum ClockMode {
  RUN,
  SET_TIME,
  SET_DATE,
  SET_DATE_ALARM_1,
  SET_DATE_ALARM_2,
  SET_DATE_ALARM_3,
  SET_LABEL_ALARM_3,
  ALARM_ON,
};

// Buzzer Song Enum
enum BuzzerSong {
  ALARM,
  CLICK,
};

// Button State Enum
enum ButtonState {
  NONE,
  PRESSED,
  HOLD,
  RELEASED,
};

// Allowed Char
char allowedChar[] = {
  " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
};
// ================== End Define Object and Constant ==================



// ============================ Song Bank =============================
// Alarm Song
int alarmSong[] = {
  2048, 0, 2048, 0, 2048, 0, 2048, 0,
0};
int alarmSongDuration[] = {0,
  90, 90, 90, 90, 90, 90, 90, 500,
};
int alarmSongSize = 9;

// Change Mode Song
int changeModeSong[] = {
  512,
0};
int changeModeSongDuration[] = {0,
  90,
};
int changeModeSongSize = 2;
// ========================== End Song Bank ===========================



// ========================= Global Variable ==========================
// Main Clock Mode
ClockMode clockMode = RUN;

// Time Offset
long long timeOffset = 0;

// Double Click MODE_BTN
#define DOUBLE_CLICK_INTERVAL 500
unsigned long doubleClickModeBtnPrevMil = 0;

// Blinker Variable
#define BLINK_TIME 1000
unsigned long blinkOffset = 0;

// Alarm Variable
bool isAlarm1On = false;
bool isAlarm2On = false;
bool isAlarm3On = false;

DateTime alarm1 = DateTime(2023,1,1,0,0,0);
DateTime alarm2 = DateTime(2022,1,1,0,0,0);
DateTime alarm3 = DateTime(2021,1,1,0,0,0);

char alarm3Label[70] = "Alarm 3";

char alarmLabel[70] = "";
// ======================= End Global Variable ========================



// ========================= Button Function ==========================
// Detect Button Input
#define HOLD_TIME_DELAY 1000
#define HOLD_PRESS_SPEED 150

bool isModeBtnDownRealTime = false;
bool isUpBtnDownRealTime = false;
bool isDownBtnDownRealTime = false;

ButtonState modeBtnState = NONE;
ButtonState upBtnState = NONE;
ButtonState downBtnState = NONE;

unsigned long modeBtnHoldPrevMil = 0;
unsigned long upBtnHoldPrevMil = 0;
unsigned long downBtnHoldPrevMil = 0;

unsigned long modeBtnHoldPressPrevMil = 0;
unsigned long upBtnHoldPressPrevMil = 0;
unsigned long downBtnHoldPressPrevMil = 0;

bool modeBtnHoldOnce = false;
bool upBtnHoldOnce = false;
bool downBtnHoldOnce = false;

int lastBtnPressed = 0;

void DetectButtonInput() {
  isModeBtnDownRealTime = digitalRead(MODE_BTN_PIN) == LOW ? true : false;
  isUpBtnDownRealTime = digitalRead(UP_BTN_PIN) == LOW ? true : false;
  isDownBtnDownRealTime = digitalRead(DOWN_BTN_PIN) == LOW ? true : false;

  // Mode Button
  if (isModeBtnDownRealTime) {
    if (modeBtnState == NONE) {
      modeBtnState = PRESSED;
      lastBtnPressed = MODE_BTN_PIN;
      modeBtnHoldPrevMil = millis();
    } else if (modeBtnState == PRESSED){
      modeBtnState = HOLD;
    }
  } else {
    if (modeBtnState == PRESSED || modeBtnState == HOLD) {
      modeBtnState = RELEASED;
    } else if (modeBtnState == RELEASED) {
      modeBtnState = NONE;
    }
  }

  // Up Button
  if (isUpBtnDownRealTime) {
    if (upBtnState == NONE) {
      upBtnState = PRESSED;
      lastBtnPressed = UP_BTN_PIN;
      upBtnHoldPrevMil = millis();
    } else if (upBtnState == PRESSED){
      upBtnState = HOLD;
    }
  } else {
    if (upBtnState == PRESSED || upBtnState == HOLD) {
      upBtnState = RELEASED;
    } else if (upBtnState == RELEASED) {
      upBtnState = NONE;
    }
  }

  // Down Button
  if (isDownBtnDownRealTime) {
    if (downBtnState == NONE) {
      downBtnState = PRESSED;
      lastBtnPressed = DOWN_BTN_PIN;
      downBtnHoldPrevMil = millis();
    } else if (downBtnState == PRESSED){
      downBtnState = HOLD;
    }
  } else {
    if (downBtnState == PRESSED || downBtnState == HOLD) {
      downBtnState = RELEASED;
    } else if (downBtnState == RELEASED) {
      downBtnState = NONE;
    }
  }
}

bool GetButtonDown(uint8_t input) {
  switch (input) {
    case MODE_BTN_PIN:
      if (modeBtnState == HOLD) {
        return true;
      }
      break;
    case UP_BTN_PIN:
      if (upBtnState == HOLD) {
        return true;
      }
      break;
    case DOWN_BTN_PIN:
      if (downBtnState == HOLD) {
        return true;
      }
      break;
  }
  return false;
}

bool GetButtonPressed(uint8_t input) {
  switch (input) {
    case MODE_BTN_PIN:
      if (modeBtnState == PRESSED) {
        return true;
      }
      break;
    case UP_BTN_PIN:
      if (upBtnState == PRESSED) {
        return true;
      }
      break;
    case DOWN_BTN_PIN:
      if (downBtnState == PRESSED) {
        return true;
      }
      break;
  }
  return false;
}

bool GetButtonHold(uint8_t input, bool once = false) {
  switch (input) {
    case MODE_BTN_PIN:
      if (modeBtnState == HOLD) {
        if (millis() > modeBtnHoldPrevMil + HOLD_TIME_DELAY) {
          if (!once) {
            if (millis() > modeBtnHoldPressPrevMil + HOLD_PRESS_SPEED) {
              modeBtnHoldPressPrevMil = millis();
              return true;
            }
          } else {
            if (!modeBtnHoldOnce) {
              modeBtnHoldOnce = true;
              return true;
            }
          }
        }
      }
      if (modeBtnState == NONE) {
        modeBtnHoldOnce = false;
      }
      break;
    case UP_BTN_PIN:
      if (upBtnState == HOLD) {
        if (millis() > upBtnHoldPrevMil + HOLD_TIME_DELAY) {
          if (!once) {
            if (millis() > upBtnHoldPressPrevMil + HOLD_PRESS_SPEED) {
              upBtnHoldPressPrevMil = millis();
              return true;
            }
          } else {
            if (!upBtnHoldOnce) {
              upBtnHoldOnce = true;
              return true;
            }
          }
        }
      }
      if (upBtnState == NONE) {
        upBtnHoldOnce = false;
      }
      break;
    case DOWN_BTN_PIN:
      if (downBtnState == HOLD) {
        if (millis() > downBtnHoldPrevMil + HOLD_TIME_DELAY) {
          if (!once) {
            if (millis() > downBtnHoldPressPrevMil + HOLD_PRESS_SPEED) {
              downBtnHoldPressPrevMil = millis();
              return true;
            }
          } else {
            if (!downBtnHoldOnce) {
              downBtnHoldOnce = true;
              return true;
            }
          }
        }
      }
      if (downBtnState == NONE) {
        downBtnHoldOnce = false;
      }
      break;
  }
  return false;
}
// ======================= End Button Function ========================



// ========================= Custom Function ==========================
// Set Display Intensity
#define setDisplayIntensityInterval 100
void SetDisplayIntensity() {
  if (millis() % 500 == 0) {
    display.setIntensity(pow(1.01, analogRead(LDR_PIN)-3910));
  }
}

// Get Current Time String
char timeBuffer[9];
const char* GetCurrentTimeString(bool removeHour = false, bool removeMinute = false, bool removeSecond = false) {
  DateTime now = rtc.now();
  now = now + TimeSpan(timeOffset);
  
  if (removeHour) {
    sprintf(timeBuffer, "??:%02d %02d", now.minute(), now.second());
    timeBuffer[3] += 80;
    timeBuffer[4] += 80;
    timeBuffer[6] += 90;
    timeBuffer[7] += 90;
  } else if (removeMinute) {
    sprintf(timeBuffer, "%02d:?? %02d", now.hour(), now.second());
    timeBuffer[0] += 80;
    timeBuffer[1] += 80;
    timeBuffer[6] += 90;
    timeBuffer[7] += 90;
  } else if (removeSecond) {
    sprintf(timeBuffer, "%02d:%02d @@", now.hour(), now.minute());
    timeBuffer[0] += 80;
    timeBuffer[1] += 80;
    timeBuffer[3] += 80;
    timeBuffer[4] += 80;
  } else {
    sprintf(timeBuffer, "%02d:%02d %02d", now.hour(), now.minute(), now.second());
    timeBuffer[0] += 80;
    timeBuffer[1] += 80;
    timeBuffer[3] += 80;
    timeBuffer[4] += 80;
    timeBuffer[6] += 90;
    timeBuffer[7] += 90;
  }

  return timeBuffer;
}

// Get Current Date String
char dateBuffer[12];
const char* GetCurrentDateString(bool removeDay = false, bool removeMonth = false, bool removeYear = false) {
  DateTime now = rtc.now();
  now = now + TimeSpan(timeOffset);
  
  if (removeDay) {
    sprintf(dateBuffer, "@@ %02s %02d", MonthShortStr(now.month()), now.year()%100);
  } else if (removeMonth) {
    sprintf(dateBuffer, "%02d ?@@ %02d", now.day(), now.year()%100);
  } else if (removeYear) {
    sprintf(dateBuffer, "%02d %02s @@", now.day(), MonthShortStr(now.month()));
  } else {
    sprintf(dateBuffer, "%02d %02s %02d", now.day(), MonthShortStr(now.month()), now.year()%100);
  }

  return dateBuffer;
}

// Get Current Temperature
char temperatureBuffer[12];
void FetchTemperature() {
  if (dht.getData()) {
    float tempDeg = dht.getTemperature();
    sprintf(temperatureBuffer, "%0.1lf°C", tempDeg);
    temperatureBuffer[0] += 80;
    temperatureBuffer[1] += 80;
    temperatureBuffer[3] += 80;
    temperatureBuffer[4] += 10;
    temperatureBuffer[6] += 112;
  }
}
const char* GetCurrentTemperatureString() {
  return temperatureBuffer;
}

// Play Buzzer
int *songPtr;
int *songDurationsPtr;
int songSize;

bool isBuzzerPlay = false;
unsigned long buzzerPrevMil = 0;
int currentNote = 0;
void BuzzerLoop() {
  if (isBuzzerPlay) {
    if (millis() - buzzerPrevMil >= songDurationsPtr[currentNote]) {
      if (songPtr[currentNote] == 0) {
        noTone(BUZZER_PIN);
      } else {
        tone(BUZZER_PIN, songPtr[currentNote]);
      }

      buzzerPrevMil = millis();
      currentNote++;

      if (currentNote >= songSize) {
        // Reset
        noTone(BUZZER_PIN);
        isBuzzerPlay = false;
        currentNote = 0;
      }
    }
  }
}
void PlayBuzzer(BuzzerSong song) {
  if (isBuzzerPlay) return;
  
  switch (song) {
    case ALARM:
      isBuzzerPlay = true;
      songPtr = alarmSong;
      songDurationsPtr = alarmSongDuration;
      songSize = alarmSongSize;
      break;
    
    case CLICK:
      isBuzzerPlay = true;
      songPtr = changeModeSong;
      songDurationsPtr = changeModeSongDuration;
      songSize = changeModeSongSize;
      break;

    default:
      break;
  }
}

// Month int to String
const char* MonthShortStr(int month) {
  switch (month) {
    case 1: return "Jan";
    case 2: return "Feb";
    case 3: return "Mar";
    case 4: return "Apr";
    case 5: return "May";
    case 6: return "Jun";
    case 7: return "Jul";
    case 8: return "Aug";
    case 9: return "Sep";
    case 10: return "Oct";
    case 11: return "Nov";
    case 12: return "Dec";
    default: return "Err";
  }
}

// Get index of some char in a string
int indexOfChar(const char* str, char target) {
  const char* ptr = strchr(str, target);
  if (ptr != NULL) {
    return ptr - str;
  } else {
    return -1;
  }
}

int stringLength(const char arr[]) {
  int length = 0;
  while (arr[length] != '\0') {
    length++;
  }
  return length;
}
// ======================= End Custom Function ========================



// ========================== Main Function ===========================
// Run Mode
uint8_t runTransitionIndex = 0;
unsigned long runMil = 0;
void Run() {
  if (display.displayAnimate()) {
    // Opening Time
    if (runTransitionIndex == 0) {
      runMil = millis();
      runTransitionIndex += 1;
      display.displayText(GetCurrentTimeString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
    }
    
    // Stay Time
    else if (runTransitionIndex == 1) {
      display.print(GetCurrentTimeString());      

      // Closing Time
      if ((millis() - runMil) >= 10000) {
        runTransitionIndex += 1;
        display.displayText(GetCurrentTimeString(), PA_CENTER, 25, 0, PA_PRINT, PA_OPENING_CURSOR);
      }
    }

    // Opening Date
    else if (runTransitionIndex == 2) {
      runTransitionIndex += 1;
      display.displayText(GetCurrentDateString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
    }

    // Stay Date
    else if (runTransitionIndex == 3) {
      display.print(GetCurrentDateString());

      // Closing Date
      if ((millis() - runMil) >= 13000) {
        runTransitionIndex += 1;
        display.displayText(GetCurrentDateString(), PA_CENTER, 25, 0, PA_PRINT, PA_OPENING_CURSOR);
        
        // Fetch DHT Data
        FetchTemperature();
      }
    }

    // Opening Temperature
    else if (runTransitionIndex == 4) {
      runTransitionIndex += 1;
      display.displayText(GetCurrentTemperatureString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
    }

    // Stay Temperature
    else if (runTransitionIndex == 5) {
      display.print(GetCurrentTemperatureString());

      // Closing Temperature
      if ((millis() - runMil) >= 16000) {
        runTransitionIndex += 1;
        display.displayText(GetCurrentTemperatureString(), PA_CENTER, 25, 0, PA_PRINT, PA_OPENING_CURSOR);
      }
    }

    // Opening Time
    else if (runTransitionIndex == 6) {
      runTransitionIndex += 1;
      display.displayText(GetCurrentTimeString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
    }

    // Stay Time
    else if (runTransitionIndex == 7) {
      display.print(GetCurrentTimeString());

      // Reset Cycle
      if ((millis() - runMil) >= 30000) {
        runMil = millis();
        runTransitionIndex = 1;
      }
    }
  }
}



// Set Time Mode
bool isTimeModeAuto = true;
int setTimeStep = 0;
void SetTime() {
  switch (setTimeStep) {
    case 0:
      {
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
          display.print(isTimeModeAuto ? "Auto:On@" : "Auto:Off");
        } else {
          display.print("Auto:@@@");
        }
      }
      break;
    
    case 1:
      {
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
          display.print(GetCurrentTimeString());
        } else {
          display.print(GetCurrentTimeString(true));
        }
      }
      break;
    
    case 2:
      {
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
          display.print(GetCurrentTimeString());
        } else {
          display.print(GetCurrentTimeString(false, true));
        }
      }
      break;
    
    case 3:
      {
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
          display.print(GetCurrentTimeString());
        } else {
          display.print(GetCurrentTimeString(false, false, true));
        }
      }
      break;
    
    default:
      display.print("STM Err");
      break;
  }

  if (GetButtonPressed(MODE_BTN_PIN) && !isTimeModeAuto) {
    blinkOffset = millis();
    PlayBuzzer(CLICK);
    setTimeStep = setTimeStep < 3 ? setTimeStep + 1 : 0;
  }

  if (GetButtonPressed(UP_BTN_PIN) || GetButtonPressed(DOWN_BTN_PIN) || GetButtonHold(UP_BTN_PIN) || GetButtonHold(DOWN_BTN_PIN)) {
    blinkOffset = millis();
    PlayBuzzer(CLICK);
    switch (setTimeStep) {
      case 0:
        isTimeModeAuto = !isTimeModeAuto;
        if (isTimeModeAuto) {
          timeOffset = 0;
        }
        break;
      case 1:
        timeOffset += lastBtnPressed == UP_BTN_PIN ? 3600 : -3600;
        break;
      case 2:
        timeOffset += lastBtnPressed == UP_BTN_PIN ? 60 : -60;
        break;
      case 3:
        timeOffset += lastBtnPressed == UP_BTN_PIN ? 1 : -1;
        break;
    }
  }
}



// Set Date Mode
int setDateStep = 0;
void SetDate() {
  switch (setDateStep) {    
    case 0:
      {
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
          display.print(GetCurrentDateString());
        } else {
          display.print(GetCurrentDateString(true));
        }
      }
      break;
    
    case 1:
      {
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
          display.print(GetCurrentDateString());
        } else {
          display.print(GetCurrentDateString(false, true));
        }
      }
      break;
    
    case 2:
      {
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
          display.print(GetCurrentDateString());
        } else {
          display.print(GetCurrentDateString(false, false, true));
        }
      }
      break;
    
    default:
      display.print("SDM Err");
      break;
  }

  if (GetButtonPressed(MODE_BTN_PIN)) {
    blinkOffset = millis();
    PlayBuzzer(CLICK);
    setDateStep = setDateStep < 2 ? setDateStep + 1 : 0;
  }

  if (GetButtonPressed(UP_BTN_PIN) || GetButtonPressed(DOWN_BTN_PIN) || GetButtonHold(UP_BTN_PIN) || GetButtonHold(DOWN_BTN_PIN)) {
    blinkOffset = millis();
    PlayBuzzer(CLICK);
    switch (setDateStep) {
      case 0:
        timeOffset += lastBtnPressed == UP_BTN_PIN ? 86400 : -86400;
        break;
      case 1:
        timeOffset += lastBtnPressed == UP_BTN_PIN ? 2592000 : -2592000;
        break;
      case 2:
        timeOffset += lastBtnPressed == UP_BTN_PIN ? 31104000 : -31104000;
        break;
    }
  }
}



// Set Alarm
bool *isOnPtr;
DateTime *alarmPtr;
int alarmIndex = 1;
int setAlarmStep = 0;
void SetAlarm() {
  switch (setAlarmStep) {
    case 0:
      {
        if (!*isOnPtr) {
          // Blinker
          if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
            char alarmText[10];
            sprintf(alarmText, "Al%d Off>@", alarmIndex);
            display.print(alarmText);
          } else {
            char alarmText[10];
            sprintf(alarmText, "Al%d @@>@@", alarmIndex);
            display.print(alarmText);
          }
        } else {
          // Blinker
          if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
            char alarmText[10];
            sprintf(alarmText, "Al%d %02d:%02d", alarmIndex, (*alarmPtr).hour(), (*alarmPtr).minute());

            display.print(alarmText);
          } else {
            char alarmText[10];
            sprintf(alarmText, "Al%d @@:%02d", alarmIndex, (*alarmPtr).minute());

            display.print(alarmText);
          }
        }
      }
      break;
    
    case 1:
      {
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
          char alarmText[10];
          sprintf(alarmText, "Al%d %02d:%02d", alarmIndex, (*alarmPtr).hour(), (*alarmPtr).minute());

          display.print(alarmText);
        } else {
          char alarmText[10];
          sprintf(alarmText, "Al%d %02d:@@", alarmIndex, (*alarmPtr).hour());

          display.print(alarmText);
        }
      }
      break;
  }

  if (GetButtonPressed(UP_BTN_PIN) || GetButtonHold(UP_BTN_PIN) || GetButtonPressed(DOWN_BTN_PIN) || GetButtonHold(DOWN_BTN_PIN)) {
    blinkOffset = millis();
    PlayBuzzer(CLICK);
    switch (setAlarmStep) {
      case 0:
        
        break;
      case 1:
        
        break;
    }
    if (!*isOnPtr) {
      *isOnPtr = true;
    } else {
      if (lastBtnPressed == UP_BTN_PIN) {
        if ((*alarmPtr).hour() >= 23) {
          *isOnPtr = false;
        }
        (*alarmPtr) = setAlarmStep == 0 ? (*alarmPtr) + TimeSpan(3600) : (*alarmPtr) + TimeSpan(60);
      } else {
        if ((*alarmPtr).hour() <= 0) {
          *isOnPtr = false;
        }
        (*alarmPtr) = setAlarmStep == 0 ? (*alarmPtr) - TimeSpan(3600) : (*alarmPtr) - TimeSpan(60);
      }
    }
  }

  if (GetButtonPressed(MODE_BTN_PIN) && *isOnPtr) {
    blinkOffset = millis();
    PlayBuzzer(CLICK);
    setAlarmStep = (setAlarmStep + 1)%2;
  }
}



// Set Label Alarm 3
int changeIndex = 0;
int displayedOffset = 0;
void SetLabelAlarm3() {
  displayedOffset = changeIndex > 4 ? changeIndex - 4 : 0;

  char displayedChar[12];
  for (int i = 0; i < 11; i++) {
    displayedChar[i] = alarm3Label[i+displayedOffset];
  }
  // Blinker
  if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
    display.print(displayedChar);
  } else {
    char temp[70];
    sprintf(temp, displayedChar);
    if (temp[changeIndex-displayedOffset] == 32) {
      temp[changeIndex-displayedOffset] = '.';
    } else if (temp[changeIndex-displayedOffset] == 'I') {
      temp[changeIndex-displayedOffset] = 44;
    } else if (temp[changeIndex-displayedOffset] == 'l' || temp[changeIndex-displayedOffset] == 'i') {
      temp[changeIndex-displayedOffset] = 45;
    } else if ((temp[changeIndex-displayedOffset] >= 48 && temp[changeIndex-displayedOffset] <= 90) || temp[changeIndex-displayedOffset] == 'm') {
      temp[changeIndex-displayedOffset] = 42;
    } else if (temp[changeIndex-displayedOffset] >= 97 && temp[changeIndex-displayedOffset] <= 122) {
      temp[changeIndex-displayedOffset] = 43;
    }
    display.print(temp);
  }

  if (GetButtonPressed(MODE_BTN_PIN)) {
    blinkOffset = millis();
    PlayBuzzer(CLICK);
    if (alarm3Label[changeIndex] != '\0') {
      changeIndex += changeIndex != 69 ? 1 : 0;
    }
  }

  if (GetButtonPressed(UP_BTN_PIN) || GetButtonHold(UP_BTN_PIN)) {
    blinkOffset = millis();
    PlayBuzzer(CLICK);
    if (alarm3Label[changeIndex] == '\0') {
      alarm3Label[changeIndex] = allowedChar[0];
    } else {
      int currentCharIndex = indexOfChar(allowedChar, alarm3Label[changeIndex]);
      alarm3Label[changeIndex] = allowedChar[currentCharIndex != 62 ? currentCharIndex + 1 : 0];
    }
  }

  if (GetButtonPressed(DOWN_BTN_PIN) || GetButtonHold(DOWN_BTN_PIN)) {
    blinkOffset = millis();
    PlayBuzzer(CLICK);
    if (alarm3Label[changeIndex] == '\0') {
      alarm3Label[changeIndex] = allowedChar[0];
    } else {
      int currentCharIndex = indexOfChar(allowedChar, alarm3Label[changeIndex]);
      alarm3Label[changeIndex] = allowedChar[currentCharIndex != 0 ? currentCharIndex - 1 : 62];
    }
  }
}



// Play Alarm
void PlayAlarm() {
  if (display.displayAnimate()) {
    display.displayClear();
    display.displayText(alarmLabel, PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }

  PlayBuzzer(ALARM);

  if (GetButtonPressed(MODE_BTN_PIN) || GetButtonPressed(UP_BTN_PIN) || GetButtonPressed(DOWN_BTN_PIN)) {
    display.displayClear();
    display.displayText("", PA_CENTER, 0, 0, PA_NO_EFFECT, PA_NO_EFFECT);
    display.setFont(tinyFont);
    ChangeClockMode(RUN);
  }
}



// Change Clock Mode
void ChangeClockMode(ClockMode newMode) {
  if (newMode != ALARM_ON) {
    PlayBuzzer(CLICK);
  }
  switch(newMode) {
    case RUN:
      runTransitionIndex = 0;
      display.setFont(tinyFont);
      break;
    case SET_TIME:
      blinkOffset = millis();
      setTimeStep = 0;
      break;
    case SET_DATE:
      blinkOffset = millis();
      setDateStep = 0;
      break;
    case SET_DATE_ALARM_1:
      blinkOffset = millis();
      setAlarmStep = 0;
      isOnPtr = &isAlarm1On;
      alarmPtr = &alarm1;
      alarmIndex = 1;
      break;
    case SET_DATE_ALARM_2:
      blinkOffset = millis();
      setAlarmStep = 0;
      isOnPtr = &isAlarm2On;
      alarmPtr = &alarm2;
      alarmIndex = 2;
      break;
    case SET_DATE_ALARM_3:
      blinkOffset = millis();
      setAlarmStep = 0;
      isOnPtr = &isAlarm3On;
      alarmPtr = &alarm3;
      alarmIndex = 3;
      break;
    case SET_LABEL_ALARM_3:
      blinkOffset = millis();
      display.setFont(bigFont);
      display.setTextAlignment(PA_LEFT);
      changeIndex = 0;
      break;
    case ALARM_ON:
      display.displayClear();
      display.setFont(nullptr);
      display.displayText("", PA_CENTER, 0, 0, PA_NO_EFFECT, PA_NO_EFFECT);
      break;
    default:
      display.print("CCM Err");
      break;
  }
  clockMode = newMode;
}
// ======================== End Main Function =========================



// =========================== Main Program ===========================
// Setup
void setup() {
  // Serial
  Serial.begin(115200);

  // Display MAX7219
  display.begin();
  display.setFont(tinyFont);
  display.setIntensity(0);
  display.setTextAlignment(PA_CENTER);
  display.setCharSpacing(1);
  // display.print("Disp On");
  
  // RTC
  // if (! rtc.isrunning()) {
  //   Serial.println("RTC is NOT running, let's set the time!");
  //   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)) + TimeSpan(10));
  // }

  if (!rtc.begin()) {
    display.print("RTC Err");
    Serial.flush();
    while (1) delay(10);
  }

  // DHT
  dht.begin();

  // Button
  pinMode(MODE_BTN_PIN, INPUT_PULLUP);
  pinMode(UP_BTN_PIN, INPUT_PULLUP);
  pinMode(DOWN_BTN_PIN, INPUT_PULLUP);
}


int count = 0;
// Loop
void loop() {
  // Detect Button Input
  DetectButtonInput();

  // Main Clock Program Loop
  switch(clockMode) {
    case RUN:
      Run();
      break;
    case SET_TIME:
      SetTime();
      break;
    case SET_DATE:
      SetDate();
      break;
    case SET_DATE_ALARM_1:
      SetAlarm();
      break;
    case SET_DATE_ALARM_2:
      SetAlarm();
      break;
    case SET_DATE_ALARM_3:
      SetAlarm();
      break;
    case SET_LABEL_ALARM_3:
      SetLabelAlarm3();
      break;
    case ALARM_ON:
      PlayAlarm();
      break;
    default:
      display.print("Mode Err");
      break;
  }

  // Detect Long Press go to Next Mode
  if (GetButtonHold(MODE_BTN_PIN, true)) {
    ChangeClockMode(clockMode != SET_LABEL_ALARM_3 ? (enum ClockMode)(clockMode + 1) : RUN);
  }

  // Detect Double Click reset to Run Mode
  if (clockMode != RUN && GetButtonPressed(MODE_BTN_PIN)) {
    if (millis() - doubleClickModeBtnPrevMil < DOUBLE_CLICK_INTERVAL) {
      ChangeClockMode(RUN);
    } else {
      doubleClickModeBtnPrevMil = millis();
    }
  }

  // Detect Alarm only if on Run Mode
  if (clockMode == RUN) {
    DateTime now = rtc.now();
    now = now + TimeSpan(timeOffset);
    if (isAlarm1On && now.hour() == alarm1.hour() && now.minute() == alarm1.minute() && now.second() == 0) {
      sprintf(alarmLabel, "5024211004");
      ChangeClockMode(ALARM_ON);
    } else if (isAlarm2On && now.hour() == alarm2.hour() && now.minute() == alarm2.minute() && now.second() == 0) {
      sprintf(alarmLabel, "5024211004 | Kenanya Keandra Adriel Prasetyo");
      ChangeClockMode(ALARM_ON);
    } else if (isAlarm3On && now.hour() == alarm3.hour() && now.minute() == alarm3.minute() && now.second() == 0) {
      sprintf(alarmLabel, alarm3Label);
      ChangeClockMode(ALARM_ON);
    }
  }

  // Auto Brightness
  SetDisplayIntensity();

  // Buzzer Loop
  BuzzerLoop();
}
// ========================= End Main Program =========================