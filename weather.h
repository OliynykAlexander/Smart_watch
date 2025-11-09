#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

class WeatherManager {
private:
  String apiKey;
  WeatherData data;
  unsigned long lastUpdate;

public:
  WeatherManager();
  
  void setApiKey(const String& key);
  String getApiKey() const { return apiKey; }
  bool hasApiKey() const { return apiKey.length() > 0; }
  
  bool fetchWeatherData();
  bool shouldUpdate() const;
  
  const WeatherData& getData() const { return data; }
  bool hasData() const { return data.hasData; }
  
  String getDescription() const { return data.description; }
  float getTemperature() const { return data.temperature; }
  int getHumidity() const { return data.humidity; }
  int getPressure() const { return data.pressure; }
};

#endif // WEATHER_H