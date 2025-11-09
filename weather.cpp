#include "weather.h"

WeatherManager::WeatherManager() : lastUpdate(0) {
  // Встановлюємо lastUpdate так, щоб перше оновлення відбулося відразу
  lastUpdate = millis() - WEATHER_UPDATE_INTERVAL;
}

void WeatherManager::setApiKey(const String& key) {
  apiKey = key;
  // При зміні API ключа скидаємо lastUpdate для негайного оновлення
  lastUpdate = millis() - WEATHER_UPDATE_INTERVAL;
}

bool WeatherManager::shouldUpdate() const {
  // Не оновлюємо, якщо немає API ключа
  if (apiKey.length() == 0) {
    return false;
  }
  
  return (millis() - lastUpdate >= WEATHER_UPDATE_INTERVAL);
}

bool WeatherManager::fetchWeatherData() {
  if (apiKey.length() == 0) {
    return false; // Не робимо запит без ключа
  }

  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=";
  url += WEATHER_CITY;
  url += "&appid=";
  url += apiKey;
  url += "&units=metric";
  url += "&lang=en";

  http.begin(url);
  http.setTimeout(10000); // Таймаут 10 секунд
  int httpCode = http.GET();

  bool success = false;

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();

      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        // Оновлюємо дані про погоду
        data.description = doc["weather"][0]["description"].as<String>();
        data.temperature = doc["main"]["temp"];
        data.humidity = doc["main"]["humidity"];
        data.pressure = doc["main"]["pressure"];
        data.hasData = true;
        data.lastUpdate = millis();

        lastUpdate = millis();
        success = true;
      }
    }
  }

  http.end();

  // Якщо запит не вдався, не оновлюємо lastUpdate
  if (!success) {
    lastUpdate = millis() - WEATHER_UPDATE_INTERVAL + 30000;
  }

  return success;
}