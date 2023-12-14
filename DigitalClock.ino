#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <RTClib.h>
#include <Bonezegei_DHT11.h>
#include <string.h>
#include "tinyFont.h"
#include "bigFont.h"

// ==================== Define Sensor and Constant ====================
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

// Battery Level
#define BATTERY_LEVEL_PIN 35

// Power Indicator
#define POWER_INDICATOR_PIN 32
// ================== End Define Sensor and Constant ==================


// =========================== Memory Bank ============================
// Alarm Song
int alarmSound[] = {2048, 0, 2048, 0, 2048, 0, 2048, 0, 0};
int alarmSoundDuration[] = {0, 90, 90, 90, 90, 90, 90, 90, 500};
int alarmSoundSize = 9;

// Change Mode Song
int changeModeSound[] = {512, 0};
int changeModeSongDuration[] = {0, 90};
int changeModeSoundSize = 2;

// Allowed Customisable Char
const char allowedChar[] = {" ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"};

// Days in Month
const int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
// ========================= End Memory Bank ==========================


// ========================= Struct and Enum ==========================
// Alarm Struct
struct Alarm {int hour; int minute;};

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
  BAT_SAVER,
};

// Buzzer Song Enum
enum BuzzerSong {ALARM, CLICK};

// Button State Enum
enum ButtonState {NONE, PRESSED, HOLD, RELEASED};
// ======================= End Struct and Enum ========================


// ========================= Global Variable ==========================
// Main Clock Mode
ClockMode clockMode = RUN;

// Blinker Variable
#define BLINK_TIME 1000
unsigned long blinkOffset = 0;

// Alarm Variable
bool isAlarm1On = false;
bool isAlarm2On = false;
bool isAlarm3On = false;
Alarm alarm1;
Alarm alarm2;
Alarm alarm3;
char alarm3Label[70] = "Alarm 3";
char alarmLabel[70] = "";

// Battery Indicator
#define BATTERY_MEAN_COUNT 1000
int batLevelIndex = 0;
int batLevelArray[BATTERY_MEAN_COUNT];
// ======================= End Global Variable ========================


// ========================= Button Function ==========================
#define HOLD_TIME_DELAY 1000
#define HOLD_PRESS_SPEED 150
#define DOUBLE_CLICK_INTERVAL 500

unsigned long doubleClickModeBtnPrevMil = 0;

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
  if (input == MODE_BTN_PIN && modeBtnState == HOLD) return true;
  if (input == UP_BTN_PIN && upBtnState == HOLD) return true;
  if (input == DOWN_BTN_PIN && downBtnState == HOLD) return true;
  return false;
}

bool GetButtonPressed(uint8_t input) {
  if (input == MODE_BTN_PIN && modeBtnState == PRESSED) return true;
  if (input == UP_BTN_PIN && upBtnState == PRESSED) return true;
  if (input == DOWN_BTN_PIN && downBtnState == PRESSED) return true;
  return false;
}

bool GetButtonHold(uint8_t input, bool once = false) {
  switch (input) {
    case MODE_BTN_PIN:
      if (modeBtnState == HOLD && millis() > modeBtnHoldPrevMil + HOLD_TIME_DELAY) {
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
      if (modeBtnState == NONE) modeBtnHoldOnce = false;
      break;
    
    case UP_BTN_PIN:
      if (upBtnState == HOLD && millis() > upBtnHoldPrevMil + HOLD_TIME_DELAY) {
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
      if (upBtnState == NONE) upBtnHoldOnce = false;
      break;
    
    case DOWN_BTN_PIN:
      if (downBtnState == HOLD && millis() > downBtnHoldPrevMil + HOLD_TIME_DELAY) {
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
      if (downBtnState == NONE) downBtnHoldOnce = false;
      break;
  }
  return false;
}
// ======================= End Button Function ========================


// ======================== Utilities Function ========================
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

// Get Battery Symbol
char getBatterySymbol(int batteryLevel) {
  if (batteryLevel <= 33) return 172;
  if (batteryLevel <= 66) return 173;
  return 174;
}

// Get index of some char in a string
int IndexOfChar(const char* str, char target) {
  const char* ptr = strchr(str, target);
  if (ptr != NULL) return ptr - str;
  else return -1;
}

// For Shift Left Array
void ShiftLeft(int arr[], int size, int positions) {
  for (int i = 0; i < positions; ++i) {
    int temp = arr[0];
    for (int j = 0; j < size - 1; ++j) arr[j] = arr[j + 1];
    arr[size - 1] = temp;
  }
}

// Clamp Function
int Clamp(int value, int min, int max) {
  if (value < min) return min;
  else if (value > max) return max;
  else return value;
}

// Leap Year
bool IsLeapYear(int year) {
  if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) return true;
  else return false;
}

// Day in Month
int DaysInMonth(int year, int month) {
  if (month == 2 && IsLeapYear(year)) return 29;
  else return daysInMonth[month];
}
// ====================== End Utilities Function ======================


// ========================= Custom Function ==========================
// Set Display Intensity
#define setDisplayIntensityInterval 100
unsigned long displayIntensityPrevMil = 0;
void SetDisplayIntensity() {
  if (millis() > displayIntensityPrevMil + setDisplayIntensityInterval) {
    displayIntensityPrevMil = millis();
    display.setIntensity(pow(1.01, analogRead(LDR_PIN)-3910));
  }
}

// Get Current Time String
char timeBuffer[9];
const char* GetCurrentTimeString(bool removeHour = false, bool removeMinute = false, bool removeSecond = false) {
  DateTime now = rtc.now();
  
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
      if (songPtr[currentNote] == 0) noTone(BUZZER_PIN);
      else tone(BUZZER_PIN, songPtr[currentNote]);

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
      songPtr = alarmSound;
      songDurationsPtr = alarmSoundDuration;
      songSize = alarmSoundSize;
      break;
    
    case CLICK:
      isBuzzerPlay = true;
      songPtr = changeModeSound;
      songDurationsPtr = changeModeSongDuration;
      songSize = changeModeSoundSize;
      break;

    default:
      break;
  }
}

// Mean Battery Level
int GetBatteryLevel() {
  int sum = 0;
  for (int i = 0; i < batLevelIndex; i++) sum += batLevelArray[i];
  return Clamp(map(sum/batLevelIndex, 1800, 2500, 0, 100), 0, 100);
}
char batteryLevelBuffer[12];
const char* GetBatteryLevelString() {
  sprintf(batteryLevelBuffer, "%c %d", getBatterySymbol(GetBatteryLevel()), GetBatteryLevel());
  return batteryLevelBuffer;
}

// Monitor Power Input
bool lastIsPowered = false;
bool isCharging = false;
#define INPUT_POWER_CHECK_INTERVAL 500
unsigned long inputPowerPrevMil;
char monitorDisplayBuffer[7];
void MonitorPowerInput() {
  if (analogRead(POWER_INDICATOR_PIN) >= 400) {
    if (!lastIsPowered) {
      lastIsPowered = true;
      inputPowerPrevMil = millis();
    }
    if (millis() > inputPowerPrevMil + INPUT_POWER_CHECK_INTERVAL) {
      if (!isCharging) {
        isCharging = true;
        sprintf(monitorDisplayBuffer, "%c  %c", getBatterySymbol(GetBatteryLevel()), 171);
        display.displayText(monitorDisplayBuffer, PA_CENTER, 50, 1000, PA_SCROLL_DOWN, PA_SCROLL_DOWN);
      }
    }
  } else {
    if (lastIsPowered) {
      lastIsPowered = false;
      inputPowerPrevMil = millis();
    }
    if (millis() > inputPowerPrevMil + INPUT_POWER_CHECK_INTERVAL) {
      if (isCharging) {
        isCharging = false;
        display.displayText(GetBatteryLevelString(), PA_CENTER, 50, 1000, PA_SCROLL_DOWN, PA_SCROLL_DOWN);
      }
    }
  }
}

// Check Battery Saver Mode
void CheckBatterySaverMode() {
  if ((analogRead(LDR_PIN) <= 400 || GetBatteryLevel() <= 20) && !isCharging) {
    ChangeClockMode(BAT_SAVER);
  }
}
// ======================= End Custom Function ========================


// ======================= Clock Mode Function ========================
// Run Mode
uint8_t runTransitionIndex = 0;
unsigned long runMil = 0;
void Run() {
  if (display.displayAnimate()) {
    // Time Information (0-10)
    if (runTransitionIndex == 0) {
      runMil = millis();
      runTransitionIndex += 1;
      display.displayText(GetCurrentTimeString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
    }
    else if (runTransitionIndex == 1) {
      display.print(GetCurrentTimeString());      
      if ((millis() - runMil) >= 10000) {
        runTransitionIndex += 1;
        display.displayText(GetCurrentTimeString(), PA_CENTER, 25, 0, PA_PRINT, PA_OPENING_CURSOR);
      }
    }
    
    // Date Information (10-13)
    else if (runTransitionIndex == 2) {
      runTransitionIndex += 1;
      display.displayText(GetCurrentDateString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
    }
    else if (runTransitionIndex == 3) {
      display.print(GetCurrentDateString());
      if ((millis() - runMil) >= 13000) {
        runTransitionIndex += 1;
        display.displayText(GetCurrentDateString(), PA_CENTER, 25, 0, PA_PRINT, PA_OPENING_CURSOR);
        // Fetch DHT Data
        FetchTemperature();
      }
    }

    // Temperature Information (13-16)
    else if (runTransitionIndex == 4) {
      runTransitionIndex += 1;
      display.displayText(GetCurrentTemperatureString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
    }
    else if (runTransitionIndex == 5) {
      display.print(GetCurrentTemperatureString());
      if ((millis() - runMil) >= 16000) {
        runTransitionIndex += 1;
        display.displayText(GetCurrentTemperatureString(), PA_CENTER, 25, 0, PA_PRINT, PA_OPENING_CURSOR);
      }
    }

    // Battery Information (16-19)
    else if (runTransitionIndex == 6) {
      runTransitionIndex += 1;
      display.displayText(GetBatteryLevelString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
    }
    else if (runTransitionIndex == 7) {
      display.print(GetBatteryLevelString());
      if ((millis() - runMil) >= 19000) {
        runTransitionIndex += 1;
        display.displayText(GetBatteryLevelString(), PA_CENTER, 25, 0, PA_PRINT, PA_OPENING_CURSOR);
      }
    }

    // Time Information Again (19-30)
    else if (runTransitionIndex == 8) {
      runTransitionIndex += 1;
      display.displayText(GetCurrentTimeString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
    }
    else if (runTransitionIndex == 9) {
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
int setTimeStep = 0;
void SetTime() {
  if (display.displayAnimate()) {
    switch (setTimeStep) {
      case 0:
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) display.print(GetCurrentTimeString());
        else display.print(GetCurrentTimeString(true));
        break;
      
      case 1:
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) display.print(GetCurrentTimeString());
        else display.print(GetCurrentTimeString(false, true));
        break;
      
      case 2:
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) display.print(GetCurrentTimeString());
        else display.print(GetCurrentTimeString(false, false, true));
        break;
    }

    if (GetButtonPressed(MODE_BTN_PIN)) {
      blinkOffset = millis();
      PlayBuzzer(CLICK);
      setTimeStep = setTimeStep < 2 ? setTimeStep + 1 : 0;
    }

    if (GetButtonPressed(UP_BTN_PIN) || GetButtonPressed(DOWN_BTN_PIN) || GetButtonHold(UP_BTN_PIN) || GetButtonHold(DOWN_BTN_PIN)) {
      blinkOffset = millis();
      PlayBuzzer(CLICK);
      if (setTimeStep == 0) rtc.adjust(rtc.now() + (lastBtnPressed == UP_BTN_PIN ? 3600 : -3600));
      else if (setTimeStep == 1) rtc.adjust(rtc.now() + (lastBtnPressed == UP_BTN_PIN ? 60 : -60));
      else if (setTimeStep == 2) rtc.adjust(rtc.now() + (lastBtnPressed == UP_BTN_PIN ? 1 : -1));
    }
  }
}

// Set Date Mode
int setDateStep = 0;
int setDateDay = 0;
int setDateMonth = 0;
int setDateYear = 0;
void SetDate() {
  if (display.displayAnimate()) {
    switch (setDateStep) {    
      case 0:
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) display.print(GetCurrentDateString());
        else display.print(GetCurrentDateString(true));
        break;
      
      case 1:
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) display.print(GetCurrentDateString());
        else display.print(GetCurrentDateString(false, true));
        break;
      
      case 2:
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) display.print(GetCurrentDateString());
        else display.print(GetCurrentDateString(false, false, true));
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

      setDateDay = rtc.now().day();
      setDateMonth = rtc.now().month();
      setDateYear = rtc.now().year();

      switch (setDateStep) {
        case 0:
          rtc.adjust(rtc.now() + (lastBtnPressed == UP_BTN_PIN ? 86400 : -86400));
          break;

        case 1:
          setDateMonth += lastBtnPressed == UP_BTN_PIN ? 1 : -1;
          if (setDateMonth > 12) {
            setDateMonth = 1;
            setDateYear += 1;
          }
          if (setDateMonth == 0) {
            setDateMonth = 12;
            setDateYear -= 1;
          }
          rtc.adjust(DateTime(setDateYear,setDateMonth,Clamp(setDateDay, 1, DaysInMonth(setDateYear, setDateMonth)), rtc.now().hour(), rtc.now().minute(), rtc.now().second()));
          break;

        case 2:
          setDateYear += lastBtnPressed == UP_BTN_PIN ? 1 : -1;
          rtc.adjust(DateTime(setDateYear,setDateMonth,Clamp(setDateDay, 1, DaysInMonth(setDateYear, setDateMonth)), rtc.now().hour(), rtc.now().minute(), rtc.now().second()));
          break;
      }
    }
  }
}

// Set Alarm
bool *isOnPtr;
Alarm *alarmPtr;
int alarmIndex = 1;
int setAlarmStep = 0;
char alarmText[10];
void SetAlarm() {
  if (display.displayAnimate()) {
    switch (setAlarmStep) {
      case 0:
        if (!*isOnPtr) {
          // Blinker
          if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
            sprintf(alarmText, "Al%d Off>@", alarmIndex);
            display.print(alarmText);
          } else {
            sprintf(alarmText, "Al%d @@>@@", alarmIndex);
            display.print(alarmText);
          }
        } else {
          // Blinker
          if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
            sprintf(alarmText, "Al%d %02d:%02d", alarmIndex, (*alarmPtr).hour, (*alarmPtr).minute);
            display.print(alarmText);
          } else {
            sprintf(alarmText, "Al%d @@:%02d", alarmIndex, (*alarmPtr).minute);
            display.print(alarmText);
          }
        }
        break;
      
      case 1:
        // Blinker
        if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
          sprintf(alarmText, "Al%d %02d:%02d", alarmIndex, (*alarmPtr).hour, (*alarmPtr).minute);
          display.print(alarmText);
        } else {
          sprintf(alarmText, "Al%d %02d:@@", alarmIndex, (*alarmPtr).hour);
          display.print(alarmText);
        }
        break;
    }

    if (GetButtonPressed(UP_BTN_PIN) || GetButtonHold(UP_BTN_PIN) || GetButtonPressed(DOWN_BTN_PIN) || GetButtonHold(DOWN_BTN_PIN)) {
      blinkOffset = millis();
      PlayBuzzer(CLICK);
      if (!*isOnPtr) {
        (*alarmPtr).hour = lastBtnPressed == UP_BTN_PIN ? 0 : 23;
        *isOnPtr = true;
      } else {
        switch (setAlarmStep) {
          case 0:
            (*alarmPtr).hour += lastBtnPressed == UP_BTN_PIN ? 1 : -1;
            if ((*alarmPtr).hour == 24 || (*alarmPtr).hour == -1) *isOnPtr = false;
            break;
          case 1:
            (*alarmPtr).minute += lastBtnPressed == UP_BTN_PIN ? 1 : -1;
            if ((*alarmPtr).minute >= 60) (*alarmPtr).minute = 0;
            else if ((*alarmPtr).minute <= -1) (*alarmPtr).minute = 59;
            break;
        }
      }
    }

    if (GetButtonPressed(MODE_BTN_PIN) && *isOnPtr) {
      blinkOffset = millis();
      PlayBuzzer(CLICK);
      setAlarmStep = (setAlarmStep + 1)%2;
    }
  }
}

// Set Label Alarm 3
int changeIndex = 0;
int displayedOffset = 0;
void SetLabelAlarm3() {
  if (display.displayAnimate()) {
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
        int currentCharIndex = IndexOfChar(allowedChar, alarm3Label[changeIndex]);
        alarm3Label[changeIndex] = allowedChar[currentCharIndex != 62 ? currentCharIndex + 1 : 0];
      }
    }

    if (GetButtonPressed(DOWN_BTN_PIN) || GetButtonHold(DOWN_BTN_PIN)) {
      blinkOffset = millis();
      PlayBuzzer(CLICK);
      if (alarm3Label[changeIndex] == '\0') {
        alarm3Label[changeIndex] = allowedChar[0];
      } else {
        int currentCharIndex = IndexOfChar(allowedChar, alarm3Label[changeIndex]);
        alarm3Label[changeIndex] = allowedChar[currentCharIndex != 0 ? currentCharIndex - 1 : 62];
      }
    }
  }
}

// Play Alarm
void PlayAlarm() {
  if (display.displayAnimate()) {
    display.displayClear();
    display.displayText(alarmLabel, PA_CENTER, 75, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  }

  PlayBuzzer(ALARM);

  if (GetButtonPressed(MODE_BTN_PIN) || GetButtonPressed(UP_BTN_PIN) || GetButtonPressed(DOWN_BTN_PIN)) {
    display.displayClear();
    display.displayText("", PA_CENTER, 0, 0, PA_NO_EFFECT, PA_NO_EFFECT);
    display.setFont(tinyFont);
    ChangeClockMode(RUN);
  }
}

// Battery Saver Mode
bool showBatSaverInfo = false;
unsigned long batSaverPrevMil = 0;
void BatterySaver() {
  if (GetButtonPressed(MODE_BTN_PIN) || GetButtonPressed(UP_BTN_PIN) || GetButtonPressed(DOWN_BTN_PIN)) {
    showBatSaverInfo = true;
    batSaverPrevMil = millis();
  }

  if (showBatSaverInfo) {
    if (millis() < batSaverPrevMil + 3000) {
      display.print(GetCurrentTimeString());
    } else if (millis() < batSaverPrevMil + 5000) {
      display.print(GetBatteryLevelString());
    } else {
      display.displayClear();
      showBatSaverInfo = false;
    }
  }

  // Exit Battery Saver Mode
  if (isCharging || analogRead(LDR_PIN) > 400) {
    display.displayText("Exit Battery Saver Mode", PA_CENTER, 25, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    while (!display.displayAnimate());
    ChangeClockMode(RUN);
  }

  delay(200);
}
// ===================== End Clock Mode Function ======================


// ======================= Change Mode Function =======================
// Change Clock Mode
void ChangeClockMode(ClockMode newMode) {
  if (newMode != ALARM_ON) PlayBuzzer(CLICK);

  switch(newMode) {
    case RUN:
      runTransitionIndex = 0;
      display.setFont(tinyFont);
      break;
    
    case SET_TIME:
      blinkOffset = millis();
      setTimeStep = 0;
      display.displayText(GetCurrentTimeString(), PA_CENTER, 25, 0, PA_SCROLL_UP, PA_PRINT);
      break;
    
    case SET_DATE:
      blinkOffset = millis();
      setDateStep = 0;
      display.displayText(GetCurrentDateString(), PA_CENTER, 25, 0, PA_SCROLL_UP, PA_PRINT);
      break;
    
    case SET_DATE_ALARM_1:
      blinkOffset = millis();
      setAlarmStep = 0;
      isOnPtr = &isAlarm1On;
      alarmPtr = &alarm1;
      alarmIndex = 1;
      sprintf(alarmText, "Al%d @@>@@", alarmIndex);
      display.displayText(alarmText, PA_CENTER, 25, 0, PA_SCROLL_UP, PA_PRINT);
      break;
    
    case SET_DATE_ALARM_2:
      blinkOffset = millis();
      setAlarmStep = 0;
      isOnPtr = &isAlarm2On;
      alarmPtr = &alarm2;
      alarmIndex = 2;
      sprintf(alarmText, "Al%d @@>@@", alarmIndex);
      display.displayText(alarmText, PA_CENTER, 25, 0, PA_SCROLL_UP, PA_PRINT);
      break;
    
    case SET_DATE_ALARM_3:
      blinkOffset = millis();
      setAlarmStep = 0;
      isOnPtr = &isAlarm3On;
      alarmPtr = &alarm3;
      alarmIndex = 3;
      sprintf(alarmText, "Al%d @@>@@", alarmIndex);
      display.displayText(alarmText, PA_CENTER, 25, 0, PA_SCROLL_UP, PA_PRINT);
      break;
    
    case SET_LABEL_ALARM_3:
      blinkOffset = millis();
      display.setFont(bigFont);
      display.setTextAlignment(PA_LEFT);
      changeIndex = 0;
      display.displayText(alarm3Label, PA_LEFT, 25, 0, PA_SCROLL_UP, PA_PRINT);
      break;
    
    case ALARM_ON:
      display.displayClear();
      display.setFont(nullptr);
      display.displayText("", PA_CENTER, 0, 0, PA_NO_EFFECT, PA_NO_EFFECT);
      break;

    case BAT_SAVER:
      noTone(BUZZER_PIN);
      display.setIntensity(0);
      display.displayClear();
      display.displayText("Enter Battery Saver Mode", PA_CENTER, 25, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      while (!display.displayAnimate());
      break;
  }
  clockMode = newMode;
}
// ===================== End Change Mode Function =====================


// =========================== Main Program ===========================
void setup() {
  // Serial
  Serial.begin(115200);

  // Display MAX7219
  display.begin();
  display.setFont(tinyFont);
  display.setIntensity(0);
  display.setTextAlignment(PA_CENTER);
  display.setCharSpacing(1);

  // RTC
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
// ========================= End Main Program =========================


// =============================== Loop ===============================
void loop() {
  // Detect Button Input
  DetectButtonInput();

  // Monitor Power Input
  MonitorPowerInput();

  // Read Battery Level
  if (batLevelIndex < BATTERY_MEAN_COUNT) {
    batLevelArray[batLevelIndex] = analogRead(BATTERY_LEVEL_PIN);
    batLevelIndex += 1;
  } else {
    ShiftLeft(batLevelArray, BATTERY_MEAN_COUNT, 1);
    batLevelArray[99] = analogRead(BATTERY_LEVEL_PIN);
  }

  // Main Clock Program Loop
  if (clockMode == RUN) Run();
  else if (clockMode == SET_TIME) SetTime();
  else if (clockMode == SET_DATE) SetDate();
  else if (clockMode == SET_DATE_ALARM_1) SetAlarm();
  else if (clockMode == SET_DATE_ALARM_2) SetAlarm();
  else if (clockMode == SET_DATE_ALARM_3) SetAlarm();
  else if (clockMode == SET_LABEL_ALARM_3) SetLabelAlarm3();
  else if (clockMode == ALARM_ON) PlayAlarm();
  else if (clockMode == BAT_SAVER) BatterySaver();
  else display.print("Mode Err");

  // Battery Saver Mode Protection
  if (clockMode == BAT_SAVER) return;

  // Detect Alarm only if on Run Mode
  if (clockMode == RUN) {
    DateTime now = rtc.now();
    if (isAlarm1On && now.hour() == alarm1.hour && now.minute() == alarm1.minute && now.second() == 0) {
      sprintf(alarmLabel, "5024211004");
      ChangeClockMode(ALARM_ON);
    } else if (isAlarm2On && now.hour() == alarm2.hour && now.minute() == alarm2.minute && now.second() == 0) {
      sprintf(alarmLabel, "5024211004 | Kenanya Keandra Adriel Prasetyo");
      ChangeClockMode(ALARM_ON);
    } else if (isAlarm3On && now.hour() == alarm3.hour && now.minute() == alarm3.minute && now.second() == 0) {
      sprintf(alarmLabel, alarm3Label);
      ChangeClockMode(ALARM_ON);
    }
  }

  // Detect Long Press go to Next Mode
  if (GetButtonHold(MODE_BTN_PIN, true)) {
    ChangeClockMode(clockMode != SET_LABEL_ALARM_3 ? (enum ClockMode)(clockMode + 1) : RUN);
  }

  // Detect Double Click reset to Run Mode
  if (clockMode != RUN && GetButtonPressed(MODE_BTN_PIN)) {
    if (millis() - doubleClickModeBtnPrevMil < DOUBLE_CLICK_INTERVAL) ChangeClockMode(RUN);
    else doubleClickModeBtnPrevMil = millis();
  }

  // Auto Brightness
  SetDisplayIntensity();

  // Buzzer Loop
  BuzzerLoop();

  // Check Battery Saver Mode
  CheckBatterySaverMode();
}
// ============================= End Loop =============================