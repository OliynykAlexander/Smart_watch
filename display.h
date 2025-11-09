#ifndef DISPLAY_H
#define DISPLAY_H

#include <LovyanGFX.hpp>
#include <Adafruit_BMP280.h>
#include <NTPClient.h>
#include <WiFi.h>
#include "config.h"
#include "weather.h"

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel;
  lgfx::Bus_SPI _bus_instance;

public:
  LGFX(void);
};

class DisplayManager {
private:
  LGFX* tft;
  LGFX_Sprite* sprite;
  Adafruit_BMP280* bmp;
  Screen* currentScreen;

  // Кеш даних для оптимізації
  String lastWeekday;
  String lastWeather;
  float lastTemperature;
  float lastPressure;
 
  void displayWeekInfo(NTPClient& timeClient);
  void displayWeatherInfo(const WeatherManager& weather);
 
public:
  DisplayManager(LGFX* display, LGFX_Sprite* spr, Adafruit_BMP280* sensor, Screen* screen);
 
  void init();
  void intro(const String& savedSSID);
  void displayApMode();
 
  Screen getCurrentScreen() const { return *currentScreen; }
 
  void displayHeader();
  void displaySetScreen(int alarmHour, int alarmMinute, bool alarmEnabled, bool alarmTriggered);
  void displayTime(NTPClient& timeClient);
  void updateTimeScreen(NTPClient& timeClient);
  void updateNatureScreen(const WeatherManager& weather);
  void updateSettingsScreen(int alarmHour, int alarmMinute, bool alarmEnabled, bool alarmTriggered);
 
  void clearScreenArea();
  void resetCache();  // Скидання кешу при зміні екрану
};

#endif // DISPLAY_H