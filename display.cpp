#include "display.h"

LGFX::LGFX(void) {
  {
    auto cfg = _bus_instance.config();
    cfg.spi_host = SPI2_HOST;
    cfg.spi_mode = 3;
    cfg.freq_write = 40000000;
    cfg.freq_read = 16000000;
    cfg.spi_3wire = true;
    cfg.use_lock = true;
    cfg.dma_channel = 1;
    cfg.pin_sclk = TFT_SCLK;
    cfg.pin_mosi = TFT_MOSI;
    cfg.pin_miso = TFT_MISO;
    cfg.pin_dc   = TFT_DC;
    _bus_instance.config(cfg);
    _panel.setBus(&_bus_instance);
  }

  {
    auto cfg = _panel.config();
    cfg.pin_cs   = TFT_CS;
    cfg.pin_rst  = TFT_RST;
    cfg.pin_busy = -1;
    cfg.panel_width  = 240;
    cfg.panel_height = 240;
    cfg.offset_x = 0;
    cfg.offset_y = 0;
    cfg.offset_rotation = 0;
    cfg.dummy_read_pixel = 8;
    cfg.dummy_read_bits  = 1;
    cfg.readable   = false;
    cfg.invert     = true;
    cfg.rgb_order  = false;
    cfg.dlen_16bit = false;
    cfg.bus_shared = true;
    _panel.config(cfg);
  }

  setPanel(&_panel);
}

DisplayManager::DisplayManager(LGFX* display, LGFX_Sprite* spr, Adafruit_BMP280* sensor, Screen* screen)
  : tft(display), sprite(spr), bmp(sensor), currentScreen(screen),
    lastWeekday(""), lastWeather(""),
    lastTemperature(0), lastPressure(-1.0) {
}

void DisplayManager::init() {
  tft->init();
  tft->setRotation(1);
  sprite->createSprite(240, 60);
  sprite->setTextSize(5);
  sprite->setTextColor(TFT_GREEN);
}

void DisplayManager::intro(const String& savedSSID) {
  tft->fillScreen(TFT_BLACK);
  sprite->fillSprite(TFT_BLACK);
  sprite->setTextColor(TFT_GREEN);
  sprite->setTextSize(2);

  if (savedSSID.length() > 0) {
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      sprite->fillScreen(TFT_BLACK);
      sprite->setCursor(0, 0);
      sprite->print("Connecting");
      for (int i = 0; i < (attempts % 4); i++) {
        sprite->print(".");
      }
      sprite->setCursor(0, 20);
      sprite->print(savedSSID);
      sprite->pushSprite(0, 80);
     
      delay(500);
      attempts++;
    }
  }

  sprite->setTextSize(5);
  sprite->setTextColor(TFT_GREEN);
}

void DisplayManager::displayApMode() {
  tft->fillScreen(TFT_BLACK);
  tft->setTextColor(TFT_GREEN);
  tft->setTextSize(2);

  tft->setCursor(0, 0);
  tft->print("AP MODE");
  tft->setCursor(0, 20);
  tft->print("SSID:");
  tft->setCursor(0, 40);
  tft->print(AP_SSID);
  tft->setCursor(0, 60);
  tft->print("Pass:");
  tft->setCursor(0, 80);
  tft->print(AP_PASSWORD);

  tft->setTextColor(TFT_YELLOW);
  tft->setCursor(0, 160);
  tft->print("Connect & setup");
  tft->setCursor(0, 180);
  tft->print("WiFi at:");
  tft->setTextColor(TFT_GREEN);
  tft->setCursor(0, 200);
  tft->print(WiFi.softAPIP().toString());
}

void DisplayManager::clearScreenArea() {
  tft->fillRect(0, 20, 240, 220, TFT_BLACK);
}

void DisplayManager::resetCache() {
  lastWeekday = "";
  lastWeather = "";
  lastTemperature = 0;
  lastPressure = -1.0;
}

void DisplayManager::displayHeader() {
  tft->fillScreen(TFT_BLACK);
 
  tft->setTextSize(2);
  uint16_t normalColor = TFT_DARKGREEN;
  uint16_t highlightColor = TFT_GREEN;
 
  tft->setTextColor(*currentScreen == SCREEN_TIME ? highlightColor : normalColor);
  tft->setCursor(0, 0);
  tft->print("TIME");

  tft->setTextColor(*currentScreen == SCREEN_NATURE ? highlightColor : normalColor);
  tft->setCursor(86, 0);
  tft->print("NATURE");

  tft->setTextColor(*currentScreen == SCREEN_SETTINGS ? highlightColor : normalColor);
  tft->setCursor(196, 0);
  tft->print("SET");

  tft->setTextColor(TFT_GREEN);
}

const char* getWeekDayName(time_t epoch) {
  struct tm* timeinfo = gmtime(&epoch);
  int wday = timeinfo->tm_wday;
  const char* days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  return days[wday];
}

void DisplayManager::displayWeekInfo(NTPClient& timeClient) {
  time_t epoch = timeClient.getEpochTime();
  const char* weekday = getWeekDayName(epoch);
  String currentWeekday = String(weekday);

  if (currentWeekday == lastWeekday) {
    return;
  }

  lastWeekday = currentWeekday;

  struct tm *timeinfo = gmtime(&epoch);
  char dateStr[20];
  sprintf(dateStr, "%02d.%02d.%04d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

  tft->fillRect(0, 90, 170, 90, TFT_BLACK);
  tft->setTextSize(3);
  tft->setCursor(0, 90);
  tft->print(weekday);

  tft->setCursor(0, 130);
  tft->print(dateStr);
}

void DisplayManager::displayWeatherInfo(const WeatherManager& weather) {
  tft->setTextSize(2);
 
  if (weather.hasData()) {
    if (lastWeather != weather.getDescription()) {
      tft->fillRect(0, 90, 200, 30, TFT_BLACK);
      tft->setCursor(0, 90);
      lastWeather = weather.getDescription();
      tft->print(weather.getDescription());
    }
    if (lastTemperature != weather.getTemperature()) {
      tft->fillRect(0, 120, 130, 30, TFT_BLACK);
      tft->setCursor(0, 120);
      lastTemperature = weather.getTemperature();
      tft->printf("Temp: %.1fC", weather.getTemperature());
    }
  }

  float mmHg = bmp->readPressure() * 0.0075006;
  if (abs(mmHg - lastPressure) >= 0.1) {
    tft->fillRect(0, 150, 130, 30, TFT_BLACK);
    tft->setCursor(0, 150);
    lastPressure = mmHg;
    tft->printf("Prss: %.1f", mmHg);
  }
}

void DisplayManager::displayTime(NTPClient& timeClient) {
  sprite->fillSprite(TFT_BLACK);
  sprite->setCursor(0, 0);

  String formattedTime = timeClient.getFormattedTime();
  sprite->print(formattedTime);

  sprite->pushSprite(0, 180);
}

void DisplayManager::displaySetScreen(int alarmHour, int alarmMinute, bool alarmEnabled, bool alarmTriggered) {
  tft->setTextSize(2);
  tft->setTextColor(TFT_GREEN);

  tft->setCursor(0, 50);
  tft->print("SERVER IP:");
  tft->setCursor(0, 80);
  tft->print(WiFi.localIP().toString());

  tft->setCursor(0, 140);
  tft->print("ALARM:");
  tft->setCursor(0, 170);
  tft->printf("Time: %02d:%02d", alarmHour, alarmMinute);
  tft->setCursor(0, 200);
  tft->print("Status: ");
  if (!alarmEnabled) {
    tft->print("DISABLED");
  } else if (alarmTriggered) {
    tft->print("Triggered");
  } else {
    tft->print("Armed");
  }
}

void DisplayManager::updateTimeScreen(NTPClient& timeClient) {
  displayWeekInfo(timeClient);
  displayTime(timeClient);
}

void DisplayManager::updateNatureScreen(const WeatherManager& weather) {
  displayWeatherInfo(weather);
}

void DisplayManager::updateSettingsScreen(int alarmHour, int alarmMinute, bool alarmEnabled, bool alarmTriggered) {
  clearScreenArea();
  displaySetScreen(alarmHour, alarmMinute, alarmEnabled, alarmTriggered);
}