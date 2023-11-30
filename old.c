// #include <MD_Parola.h>
// #include <MD_MAX72xx.h>
// #include "myFont.h"
// #include <SPI.h>
// #include "RTClib.h"
// #include <Bonezegei_DHT11.h>

// // ==================== Define Object and Constant ====================
// // Display MAX7219
// #define HARDWARE_TYPE MD_MAX72XX::FC16_HW
// #define MAX_DEVICES 4
// #define DISPLAY_CS_PIN 5
// MD_Parola display = MD_Parola(HARDWARE_TYPE, DISPLAY_CS_PIN, MAX_DEVICES);

// // RTC
// RTC_DS1307 rtc;

// // DHT
// #define DHT_PIN 14
// Bonezegei_DHT11 dht(DHT_PIN);

// // Button
// #define MODE_BTN_PIN 2
// #define UP_BTN_PIN 0
// #define DOWN_BTN_PIN 4

// // Buzzer
// #define BUZZER_PIN 33

// // LDR
// #define LDR_PIN 27

// // ClockMode Enum
// enum ClockMode {
//   RUN,
//   SET_TIME,
//   SET_DATE,
//   SET_DATE_ALARM_1,
//   SET_DATE_ALARM_2,
//   SET_DATE_ALARM_3,
//   SET_LABEL_ALARM_3,
// };

// // Buzzer Song Enum
// enum BuzzerSong {
//   ALARM,
//   CLICK,
// };
// // ================== End Define Object and Constant ==================



// // ============================ Song Bank =============================
// // Alarm Song
// int alarmSong[] = {
//   2048, 0, 2048, 0, 2048, 0, 2048, 0,
// 0};
// int alarmSongDuration[] = {0,
//   90, 90, 90, 90, 90, 90, 90, 500,
// };
// int alarmSongSize = 9;

// // Change Mode Song
// int changeModeSong[] = {
//   512,
// 0};
// int changeModeSongDuration[] = {0,
//   90,
// };
// int changeModeSongSize = 2;
// // ========================== End Song Bank ===========================



// // ========================= Global Variable ==========================
// unsigned long frame = 0;

// // Main Clock Mode
// ClockMode clockMode = RUN;

// // Time Offset
// long long timeOffset = 0;

// // Hold MODE_BTN
// #define HOLD_TIME 1000
// unsigned long holdModeBtnPrevMil = 0;
// bool doneHolding = false;

// // Double Click MODE_BTN
// #define DOUBLE_CLICK_INTERVAL 500
// unsigned long doubleClickModeBtnPrevMil = 0;

// // Blinker Variable
// #define BLINK_TIME 1000
// unsigned long blinkOffset = 0;

// // Alarm Variable
// bool isAlarm1On = false;
// bool isAlarm2On = false;
// bool isAlarm3On = false;

// // ======================= End Global Variable ========================



// // ========================= Global Function ==========================
// // Button Pressed and Down + Debounce
// #define DEBOUNCE_TIME 100
// int pressedButtonIndex = -1;
// unsigned long buttonDownPrevMil = 0;
// unsigned long buttonPressedFrame = 0;
// #define HOLD_TIME_DELAY 1000
// #define HOLD_PRESS_SPEED 100
// unsigned long buttonHoldPrevMil = 0;

// bool GetButtonDown(uint8_t input) {
//   if (pressedButtonIndex == -1) {
//     int state = digitalRead(input);
//     if (state == LOW) {
//       pressedButtonIndex = input;
//       buttonDownPrevMil = millis();
//       buttonHoldPrevMil = millis();
//       buttonPressedFrame = frame;
//       return true;
//     }
//   }
  
//   if (pressedButtonIndex == input) {
//     if ((millis() - buttonDownPrevMil) >= DEBOUNCE_TIME) {
//       int state = digitalRead(input);
//       if (state == LOW) {
//         return true;
//       } else {
//         pressedButtonIndex = -1;
//         return false;
//       }
//     } else {
//       return true;
//     }
//   }
  
//   return false;
// }

// bool GetButtonPressed(uint8_t input) {
//   bool state = GetButtonDown(input);
//   if (pressedButtonIndex == input) {
//     if (buttonPressedFrame == frame) {
//       return true;
//     }
//   }
//   return false;
// }

// bool isHoldDoneFrame = false;
// bool GetButtonHold(uint8_t input) {
//   if (pressedButtonIndex == input) {
//     if (millis() > buttonHoldPrevMil + HOLD_TIME_DELAY) {
//       if (millis() % HOLD_PRESS_SPEED == 0) {
//         if (!isHoldDoneFrame) {
//           isHoldDoneFrame = true;
//           return true;
//         }
//       } else {
//         isHoldDoneFrame = false;
//       }
//     }
//   }
//   return false;
// }

// // Month int to String
// const char* MonthShortStr(int month) {
//   switch (month) {
//     case 1: return "Jan";
//     case 2: return "Feb";
//     case 3: return "Mar";
//     case 4: return "Apr";
//     case 5: return "May";
//     case 6: return "Jun";
//     case 7: return "Jul";
//     case 8: return "Aug";
//     case 9: return "Sep";
//     case 10: return "Oct";
//     case 11: return "Nov";
//     case 12: return "Dec";
//     default: return "Err";
//   }
// }
// // ======================= End Global Function ========================



// // ========================= Custom Function ==========================
// // Detect Button Input
// void DetectButtonInput() {
  
// }

// // Set Display Intensity
// #define setDisplayIntensityInterval 500
// void SetDisplayIntensity() {
//   if (millis() % 500 == 0) {
//     display.setIntensity(pow(1.01, analogRead(LDR_PIN)-3910));
//   }
// }

// // Get Current Time String
// char timeBuffer[9];
// const char* GetCurrentTimeString(bool removeHour = false, bool removeMinute = false, bool removeSecond = false) {
//   DateTime now = rtc.now();
//   now = now + TimeSpan(timeOffset);
  
//   if (removeHour) {
//     sprintf(timeBuffer, "??:%02d %02d", now.minute(), now.second());
//     timeBuffer[3] += 80;
//     timeBuffer[4] += 80;
//     timeBuffer[6] += 90;
//     timeBuffer[7] += 90;
//   } else if (removeMinute) {
//     sprintf(timeBuffer, "%02d:?? %02d", now.hour(), now.second());
//     timeBuffer[0] += 80;
//     timeBuffer[1] += 80;
//     timeBuffer[6] += 90;
//     timeBuffer[7] += 90;
//   } else if (removeSecond) {
//     sprintf(timeBuffer, "%02d:%02d @@", now.hour(), now.minute());
//     timeBuffer[0] += 80;
//     timeBuffer[1] += 80;
//     timeBuffer[3] += 80;
//     timeBuffer[4] += 80;
//   } else {
//     sprintf(timeBuffer, "%02d:%02d %02d", now.hour(), now.minute(), now.second());
//     timeBuffer[0] += 80;
//     timeBuffer[1] += 80;
//     timeBuffer[3] += 80;
//     timeBuffer[4] += 80;
//     timeBuffer[6] += 90;
//     timeBuffer[7] += 90;
//   }

//   return timeBuffer;
// }

// // Get Current Date String
// char dateBuffer[12];
// const char* GetCurrentDateString(bool removeDay = false, bool removeMonth = false, bool removeYear = false) {
//   DateTime now = rtc.now();
//   now = now + TimeSpan(timeOffset);
  
//   if (removeDay) {
//     sprintf(dateBuffer, "@@ %s %d", MonthShortStr(now.month()), now.year()%100);
//   } else if (removeMonth) {
//     sprintf(dateBuffer, "%d @@@ %d", now.day(), now.year()%100);
//   } else if (removeYear) {
//     sprintf(dateBuffer, "%d %s @@", now.day(), MonthShortStr(now.month()));
//   } else {
//     sprintf(dateBuffer, "%d %s %d", now.day(), MonthShortStr(now.month()), now.year()%100);
//   }

//   return dateBuffer;
// }

// // Get Current Temperature
// char temperatureBuffer[12];
// void FetchTemperature() {
//   if (dht.getData()) {
//     float tempDeg = dht.getTemperature();
//     sprintf(temperatureBuffer, "%0.1lf°C", tempDeg);
//   }
// }
// const char* GetCurrentTemperatureString() {
//   return temperatureBuffer;
// }

// // Play Buzzer
// int *songPtr;
// int *songDurationsPtr;
// int songSize;

// bool isBuzzerPlay = false;
// unsigned long buzzerPrevMil = 0;
// int currentNote = 0;
// void BuzzerLoop() {
//   if (isBuzzerPlay) {
//     if (millis() - buzzerPrevMil >= songDurationsPtr[currentNote]) {
//       if (songPtr[currentNote] == 0) {
//         noTone(BUZZER_PIN);
//       } else {
//         tone(BUZZER_PIN, songPtr[currentNote]);
//       }

//       buzzerPrevMil = millis();
//       currentNote++;

//       if (currentNote >= songSize) {
//         // Reset
//         noTone(BUZZER_PIN);
//         isBuzzerPlay = false;
//         currentNote = 0;
//       }
//     }
//   }
// }
// void PlayBuzzer(BuzzerSong song) {
//   if (isBuzzerPlay) return;
  
//   switch (song) {
//     case ALARM:
//       isBuzzerPlay = true;
//       songPtr = alarmSong;
//       songDurationsPtr = alarmSongDuration;
//       songSize = alarmSongSize;
//       break;
    
//     case CLICK:
//       isBuzzerPlay = true;
//       songPtr = changeModeSong;
//       songDurationsPtr = changeModeSongDuration;
//       songSize = changeModeSongSize;
//       break;

//     default:
//       break;
//   }
// }
// // ======================= End Custom Function ========================



// // ========================== Main Function ===========================
// // Run Mode
// uint8_t runTransitionIndex = 0;
// unsigned long runMil = 0;
// void Run() {
//   if (display.displayAnimate()) {
//     // Opening Time
//     if (runTransitionIndex == 0) {
//       runMil = millis();
//       runTransitionIndex += 1;
//       display.displayText(GetCurrentTimeString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
//     }
    
//     // Stay Time
//     else if (runTransitionIndex == 1) {
//       display.print(GetCurrentTimeString());      

//       // Closing Time
//       if ((millis() - runMil) >= 10000) {
//         runTransitionIndex += 1;
//         display.displayText(GetCurrentTimeString(), PA_CENTER, 25, 0, PA_PRINT, PA_OPENING_CURSOR);
//       }
//     }

//     // Opening Date
//     else if (runTransitionIndex == 2) {
//       runTransitionIndex += 1;
//       display.displayText(GetCurrentDateString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
//     }

//     // Stay Date
//     else if (runTransitionIndex == 3) {
//       display.print(GetCurrentDateString());

//       // Closing Date
//       if ((millis() - runMil) >= 13000) {
//         runTransitionIndex += 1;
//         display.displayText(GetCurrentDateString(), PA_CENTER, 25, 0, PA_PRINT, PA_OPENING_CURSOR);
        
//         // Fetch DHT Data
//         FetchTemperature();
//       }
//     }

//     // Opening Temperature
//     else if (runTransitionIndex == 4) {
//       runTransitionIndex += 1;
//       display.displayText(GetCurrentTemperatureString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
//     }

//     // Stay Temperature
//     else if (runTransitionIndex == 5) {
//       display.print(GetCurrentTemperatureString());

//       // Closing Temperature
//       if ((millis() - runMil) >= 16000) {
//         runTransitionIndex += 1;
//         display.displayText(GetCurrentTemperatureString(), PA_CENTER, 25, 0, PA_PRINT, PA_OPENING_CURSOR);
//       }
//     }

//     // Opening Time
//     else if (runTransitionIndex == 6) {
//       runTransitionIndex += 1;
//       display.displayText(GetCurrentTimeString(), PA_CENTER, 25, 0, PA_OPENING_CURSOR, PA_PRINT);
//     }

//     // Stay Time
//     else if (runTransitionIndex == 7) {
//       display.print(GetCurrentTimeString());

//       // Reset Cycle
//       if ((millis() - runMil) >= 30000) {
//         runMil = millis();
//         runTransitionIndex = 1;
//       }
//     }
//   }
// }



// // Set Time Mode
// bool isTimeModeAuto = true;
// int setTimeStep = 0;
// void SetTime() {
//   switch (setTimeStep) {
//     case 0:
//       {
//         // Blinker
//         if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
//           display.print(isTimeModeAuto ? "Auto:On@" : "Auto:Off");
//         } else {
//           display.print("Auto:@@@");
//         }
//       }
//       break;
    
//     case 1:
//       {
//         // Blinker
//         if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
//           display.print(GetCurrentTimeString());
//         } else {
//           display.print(GetCurrentTimeString(true));
//         }
//       }
//       break;
    
//     case 2:
//       {
//         // Blinker
//         if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
//           display.print(GetCurrentTimeString());
//         } else {
//           display.print(GetCurrentTimeString(false, true));
//         }
//       }
//       break;
    
//     case 3:
//       {
//         // Blinker
//         if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
//           display.print(GetCurrentTimeString());
//         } else {
//           display.print(GetCurrentTimeString(false, false, true));
//         }
//       }
//       break;
    
//     default:
//       display.print("STM Err");
//       break;
//   }

//   if (GetButtonPressed(MODE_BTN_PIN) && !isTimeModeAuto) {
//     blinkOffset = millis();
//     PlayBuzzer(CLICK);
//     setTimeStep = setTimeStep < 3 ? setTimeStep + 1 : 0;
//   }

//   if (GetButtonPressed(UP_BTN_PIN) || GetButtonPressed(DOWN_BTN_PIN) || GetButtonHold(UP_BTN_PIN) || GetButtonHold(DOWN_BTN_PIN)) {
//     blinkOffset = millis();
//     PlayBuzzer(CLICK);
//     switch (setTimeStep) {
//       case 0:
//         isTimeModeAuto = !isTimeModeAuto;
//         if (isTimeModeAuto) {
//           timeOffset = 0;
//         }
//         break;
//       case 1:
//         timeOffset += pressedButtonIndex == UP_BTN_PIN ? 3600 : -3600;
//         break;
//       case 2:
//         timeOffset += pressedButtonIndex == UP_BTN_PIN ? 60 : -60;
//         break;
//       case 3:
//         timeOffset += pressedButtonIndex == UP_BTN_PIN ? 1 : -1;
//         break;
//     }
//   }
// }



// // Set Date Mode
// int setDateStep = 0;
// void SetDate() {
//   switch (setDateStep) {    
//     case 0:
//       {
//         // Blinker
//         if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
//           display.print(GetCurrentDateString());
//         } else {
//           display.print(GetCurrentDateString(true));
//         }
//       }
//       break;
    
//     case 1:
//       {
//         // Blinker
//         if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
//           display.print(GetCurrentDateString());
//         } else {
//           display.print(GetCurrentDateString(false, true));
//         }
//       }
//       break;
    
//     case 2:
//       {
//         // Blinker
//         if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
//           display.print(GetCurrentDateString());
//         } else {
//           display.print(GetCurrentDateString(false, false, true));
//         }
//       }
//       break;
    
//     default:
//       display.print("SDM Err");
//       break;
//   }

//   if (GetButtonPressed(MODE_BTN_PIN)) {
//     blinkOffset = millis();
//     PlayBuzzer(CLICK);
//     setDateStep = setDateStep < 2 ? setDateStep + 1 : 0;
//   }

//   if (GetButtonPressed(UP_BTN_PIN) || GetButtonPressed(DOWN_BTN_PIN) || GetButtonHold(UP_BTN_PIN) || GetButtonHold(DOWN_BTN_PIN)) {
//     blinkOffset = millis();
//     PlayBuzzer(CLICK);
//     switch (setDateStep) {
//       case 0:
//         timeOffset += pressedButtonIndex == UP_BTN_PIN ? 86400 : -86400;
//         break;
//       case 1:
//         timeOffset += pressedButtonIndex == UP_BTN_PIN ? 2592000 : -2592000;
//         break;
//       case 2:
//         timeOffset += pressedButtonIndex == UP_BTN_PIN ? 31104000 : -31104000;
//         break;
//     }
//   }
// }



// // Set Alarm 1
// int setAlarm1Step = 0;
// void SetAlarm1() {
//   switch (setAlarm1Step) {
//     case 0:
//       {
//         // Blinker
//         if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
//           display.print(GetCurrentDateString());
//         } else {
//           display.print(GetCurrentDateString(true));
//         }
//       }
//       break;
    
//     case 1:
//       {
//         // Blinker
//         if (((millis()-blinkOffset) % BLINK_TIME) / (BLINK_TIME/2) == 0) {
//           display.print(GetCurrentDateString());
//         } else {
//           display.print(GetCurrentDateString(false, true));
//         }
//       }
//       break;
//   }
// }



// // Change Clock Mode
// void ChangeClockMode(ClockMode newMode) {
//   PlayBuzzer(CLICK);
//   switch(newMode) {
//     case RUN:
//       runTransitionIndex = 0;
//       break;
//     case SET_TIME:
//       blinkOffset = millis();
//       setTimeStep = 0;
//       break;
//     case SET_DATE:
//       blinkOffset = millis();
//       setDateStep = 0;
//       break;
//     case SET_DATE_ALARM_1:
//       break;
//     case SET_DATE_ALARM_2:
//       break;
//     case SET_DATE_ALARM_3:
//       break;
//     case SET_LABEL_ALARM_3:
//       break;
//     default:
//       display.print("CCM Err");
//       break;
//   }
//   clockMode = newMode;
// }
// // ======================== End Main Function =========================



// // =========================== Main Program ===========================
// // Setup
// void setup() {
//   // Serial
//   Serial.begin(115200);

//   // Display MAX7219
//   display.begin();
//   display.setFont(myFont);
//   display.setIntensity(0);
//   display.setTextAlignment(PA_CENTER);
//   display.setCharSpacing(1);
//   // display.print("Disp On");
  
//   // RTC
//   if (!rtc.begin()) {
//     display.print("RTC Err");
//     Serial.flush();
//     while (1) delay(10);
//   }

//   // DHT
//   dht.begin();

//   // Button
//   pinMode(MODE_BTN_PIN, INPUT_PULLUP);
//   pinMode(UP_BTN_PIN, INPUT_PULLUP);
//   pinMode(DOWN_BTN_PIN, INPUT_PULLUP);
// }



// // Loop
// void loop() {
//   // Detect Button Input
//   DetectButtonInput();

//   // Main Clock Program Loop
//   switch(clockMode) {
//     case RUN:
//       Run();
//       break;
//     case SET_TIME:
//       SetTime();
//       break;
//     case SET_DATE:
//       SetDate();
//       break;
//     case SET_DATE_ALARM_1:
//       SetAlarm1();
//       display.print("AL1 13:30");
//       break;
//     case SET_DATE_ALARM_2:
//       display.print("AL2");
//       break;
//     case SET_DATE_ALARM_3:
//       display.print("AL3");
//       break;
//     case SET_LABEL_ALARM_3:
//       display.print("Lab AL3");
//       break;
//     default:
//       display.print("Mode Err");
//       break;
//   }

//   // Detect Long Press go to Next Mode
//   if (GetButtonDown(MODE_BTN_PIN)) {
//     if (GetButtonPressed(MODE_BTN_PIN)) {
//       holdModeBtnPrevMil = millis();
//     }
//     if (millis() > holdModeBtnPrevMil+HOLD_TIME && !doneHolding) {
//       ChangeClockMode(clockMode != SET_LABEL_ALARM_3 ? (enum ClockMode)(clockMode + 1) : RUN);
//       doneHolding = true;
//     }
//   } else {
//     doneHolding = false;
//   }

//   // Detect Double Click reset to Run Mode
//   if (clockMode != RUN && GetButtonPressed(MODE_BTN_PIN)) {
//     if (millis() - doubleClickModeBtnPrevMil < DOUBLE_CLICK_INTERVAL) {
//       ChangeClockMode(RUN);
//     } else {
//       doubleClickModeBtnPrevMil = millis();
//     }
//   }

//   // Emergency Alarm
//   DateTime now = rtc.now();
//   now = now + TimeSpan(timeOffset);

//   if (now.hour() == 5 && now.minute() >= 30 && now.minute() < 40) {
//     PlayBuzzer(ALARM);
//   }

//   // Auto Brightness
//   SetDisplayIntensity();

//   // Buzzer Loop
//   BuzzerLoop();

//   frame += 1;
// }
// // ========================= End Main Program =========================