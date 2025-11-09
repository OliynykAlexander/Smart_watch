#include "storage.h"

void Storage::begin() {
  preferences.begin(PREF_NAMESPACE, false);
}

// WiFi
void Storage::saveWiFiCredentials(const String& ssid, const String& password) {
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
}

String Storage::loadSSID() {
  return preferences.getString("ssid", "");
}

String Storage::loadPassword() {
  return preferences.getString("password", "");
}

// Будильник
void Storage::saveAlarmSettings(int hour, int minute, bool enabled) {
  preferences.putInt("alarm_hour", hour);
  preferences.putInt("alarm_min", minute);
  preferences.putBool("alarm_en", enabled);
}

void Storage::loadAlarmSettings(int& hour, int& minute, bool& enabled) {
  hour = preferences.getInt("alarm_hour", 9);
  minute = preferences.getInt("alarm_min", 0);
  enabled = preferences.getBool("alarm_en", true);
}

// LED
void Storage::saveLEDMode(LedMode mode) {
  preferences.putInt("led_mode", (int)mode);
}

LedMode Storage::loadLEDMode() {
  return (LedMode)preferences.getInt("led_mode", LED_OFF);
}

// Weather API
void Storage::saveWeatherApiKey(const String& apiKey) {
  preferences.putString("weather_key", apiKey);
}

String Storage::loadWeatherApiKey() {
  return preferences.getString("weather_key", "");
}