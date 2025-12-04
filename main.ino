#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Adafruit_BMP280.h>
#include <time.h>

#include "config.h"
#include "storage.h"
#include "alarm.h"
#include "weather.h"
#include "display.h"
#include "wifi_manager.h"

// ============= ГЛОБАЛЬНІ ОБ'ЄКТИ =============
Storage storage;
AlarmManager alarmManager;
WeatherManager weatherManager;

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, GMT_OFFSET_SEC, NTP_UPDATE_INTERVAL);

// Датчик
Adafruit_BMP280 bmp;

// Дисплей
LGFX tft;
LGFX_Sprite sprite(&tft);
Screen currentScreen = SCREEN_TIME;
DisplayManager displayManager(&tft, &sprite, &bmp, &currentScreen);

// WiFi і веб-сервер
bool needUpdateSetScreen = false;
WiFiManager wifiManager(&storage, &alarmManager, &weatherManager, &currentScreen, &needUpdateSetScreen);

// ============= УПРАВЛІННЯ СВІТЛОДІОДАМИ =============
LedMode ledMode = LED_OFF;

void controlLED() {
  switch (ledMode) {
    case LED_FORCE:
      digitalWrite(WHITE_LED_PIN, HIGH);
      break;
    case LED_OFF:
      digitalWrite(WHITE_LED_PIN, LOW);
      break;
  }
}

// ============= УПРАВЛІННЯ КНОПКОЮ =============
unsigned long buttonPressStart = 0;
unsigned long lastDebounceTime = 0;
bool longPressHandled = false;
int lastButtonState = HIGH;
int buttonState = HIGH;

void handleButtonPress() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading == LOW && lastButtonState == HIGH) {
    buttonPressStart = millis();
    longPressHandled = false;
  }
 
  if (reading == LOW && !longPressHandled) {
    if (millis() - buttonPressStart >= BUTTON_LONG_PRESS_TIME) {
      // Довге натискання - зміна режиму LED
      ledMode = (LedMode)((ledMode + 1) % 2);
      longPressHandled = true;
      storage.saveLEDMode(ledMode);
     
      if (currentScreen == SCREEN_SETTINGS) {
        needUpdateSetScreen = true;
      }
     
      // Мигання жовтим LED
      for (int i = 0; i < 3; i++) {
        digitalWrite(YELLOW_LED_PIN, HIGH);
        delay(100);
        digitalWrite(YELLOW_LED_PIN, LOW);
        delay(100);
      }
    }
  }

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == HIGH && !longPressHandled) {
        if (millis() - buttonPressStart < BUTTON_LONG_PRESS_TIME) {
          // Коротке натискання - зміна екрану
          currentScreen = (Screen)((currentScreen + 1) % SCREEN_COUNT);
          displayManager.clearScreenArea();
          displayManager.displayHeader();

          switch (currentScreen) {
            case SCREEN_TIME:
              timeClient.update();
              displayManager.updateTimeScreen(timeClient);
              break;
              
            case SCREEN_NATURE:
              displayManager.resetCache();
              if (!weatherManager.hasData()) {
                tft.setTextSize(2);
                tft.setTextColor(TFT_GREEN);
                tft.setCursor(0, 90);
                tft.print("Waiting for");
                tft.setCursor(0, 120);
                tft.print("weather data");
              }
              displayManager.updateNatureScreen(weatherManager);
              break;
              
            case SCREEN_SETTINGS:
              displayManager.resetCache();
              displayManager.updateSettingsScreen(
                alarmManager.getHour(),
                alarmManager.getMinute(),
                alarmManager.isEnabled(),
                alarmManager.isTriggered()
              );
              break;
          }
        }
      }
    }
  }

  lastButtonState = reading;
  digitalWrite(YELLOW_LED_PIN, (reading == LOW) ? HIGH : LOW);
}

// ============= ТАЙМЕРИ =============
unsigned long lastTimeUpdate = 0;

// ============= SETUP =============
void setup() {
  // Ініціалізація дисплея
  displayManager.init();

  // Ініціалізація Storage
  storage.begin();

  // Завантаження налаштувань
  String savedSSID = storage.loadSSID();
  String savedPassword = storage.loadPassword();
  int hour, minute;
  bool enabled;
  storage.loadAlarmSettings(hour, minute, enabled);
  alarmManager.setTime(hour, minute);
  alarmManager.setEnabled(enabled);
  ledMode = storage.loadLEDMode();
  String apiKey = storage.loadWeatherApiKey();
  weatherManager.setApiKey(apiKey);

  // Спроба підключення до WiFi
  displayManager.intro(savedSSID);
  if (savedSSID.length() > 0) {
    wifiManager.connectToWiFi(savedSSID, savedPassword);
  }

  // Якщо не підключились, запускаємо AP
  if (!wifiManager.isConnected()) {
    wifiManager.startAP();
    displayManager.displayApMode();
    wifiManager.begin();

    // Залишаємось в режимі очікування
    while (!wifiManager.isConnected()) {
      wifiManager.handleClient();
      delay(10);
    }

    // Якщо підключились через веб-панель, перезапускаємо
    ESP.restart();
  }

  // Ініціалізація периферії
  timeClient.begin();
  bmp.begin(0x76);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);
  alarmManager.setupI2S();

  // Налаштування NTP
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  // Очікування синхронізації NTP
  int syncAttempts = 0;
  while (!timeClient.update() && syncAttempts < 20) {
    delay(500);
    syncAttempts++;
  }

  // Якщо не вдалося синхронізуватися, спробувати ще раз через forceUpdate
  if (syncAttempts >= 20) {
    timeClient.forceUpdate();
    delay(1000);
  }

  // Перше оновлення погоди
  if (weatherManager.hasApiKey()) {
    weatherManager.fetchWeatherData();
  }

  // Налаштування веб-сервера
  wifiManager.begin();

  // Відображення початкового екрану
  displayManager.displayHeader();
  displayManager.updateTimeScreen(timeClient);
}

// ============= LOOP =============
void loop() {
  wifiManager.handleClient();

  if (wifiManager.isConnected()) {
    unsigned long now = millis();

    // Автоматичне оновлення погоди
    if (weatherManager.shouldUpdate()) {
      weatherManager.fetchWeatherData();
    }

    controlLED();
    handleButtonPress();

    // Оновлення дисплея
    if (now - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
      timeClient.update();
      alarmManager.checkAlarm(timeClient);
     
      switch (currentScreen) {
        case SCREEN_TIME:
          displayManager.updateTimeScreen(timeClient);
          break;
          
        case SCREEN_NATURE:
          displayManager.updateNatureScreen(weatherManager);
          break;
          
        case SCREEN_SETTINGS:
          if (needUpdateSetScreen) {
            displayManager.updateSettingsScreen(
              alarmManager.getHour(),
              alarmManager.getMinute(),
              alarmManager.isEnabled(),
              alarmManager.isTriggered()
            );
            needUpdateSetScreen = false;
          }
          break;
      }
      lastTimeUpdate = now;
    }
  } else {
    // Якщо втратили з'єднання, перезапускаємо
    ESP.restart();
  }
}