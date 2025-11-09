#ifndef STORAGE_H
#define STORAGE_H

#include <Preferences.h>
#include <Arduino.h>
#include "config.h"

class Storage {
private:
  Preferences preferences;

public:
  void begin();
  
  // WiFi
  void saveWiFiCredentials(const String& ssid, const String& password);
  String loadSSID();
  String loadPassword();
  
  // Будильник
  void saveAlarmSettings(int hour, int minute, bool enabled);
  void loadAlarmSettings(int& hour, int& minute, bool& enabled);
  
  // LED
  void saveLEDMode(LedMode mode);
  LedMode loadLEDMode();
  
  // Weather API
  void saveWeatherApiKey(const String& apiKey);
  String loadWeatherApiKey();
};

#endif // STORAGE_H