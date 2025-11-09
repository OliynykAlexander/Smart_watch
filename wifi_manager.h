#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include "config.h"
#include "storage.h"
#include "alarm.h"
#include "weather.h"

class WiFiManager {
private:
  WebServer server;
  Storage* storage;
  AlarmManager* alarmManager;
  WeatherManager* weatherManager;
  Screen* currentScreen;
  bool* needUpdateSetScreen;
  
  void handleRoot();
  void handleConnect();
  void handleStatus();
  void handleAlarm();
  void handleWeatherUpdate();
  void handleWeatherApiKey();
  void handleNotFound();

public:
  WiFiManager(Storage* stor, AlarmManager* alarm, WeatherManager* weather, 
              Screen* screen, bool* needUpdate);
  
  void begin();
  void handleClient();
  
  bool connectToWiFi(const String& ssid, const String& password);
  void startAP();
  bool isConnected();
};

#endif // WIFI_MANAGER_H