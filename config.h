#ifndef CONFIG_H
#define CONFIG_H

// ============= ВИЗНАЧЕННЯ ПІНІВ =============
// I2S піни
#define I2S_BCLK  20
#define I2S_LRC   21
#define I2S_DOUT  10

// Піни кнопки та світлодіодів
#define BUTTON_PIN 1
#define YELLOW_LED_PIN 5
#define WHITE_LED_PIN 7

// Піни дисплея SPI
#define TFT_SCLK 4
#define TFT_MOSI 6
#define TFT_MISO -1  // не використовується
#define TFT_DC   3
#define TFT_CS   -1  // не використовується
#define TFT_RST  2

// Піни BMP280
// I2C SDA 8
// I2C SCL 9

// ============= ПАРАМЕТРИ I2S =============
#define I2S_SAMPLE_RATE   16000
#define I2S_BEEP_FREQ     1000
#define I2S_BEEP_MS       500

// ============= НАЛАШТУВАННЯ WiFi =============
#define AP_SSID "ESP_Terminal"
#define AP_PASSWORD "12345678"
#define WEB_SERVER_PORT 80

// ============= НАЛАШТУВАННЯ ПОГОДИ =============
#define WEATHER_CITY "Kyiv"
#define WEATHER_UPDATE_INTERVAL 600000  // 10 хвилин

// ============= NTP НАЛАШТУВАННЯ =============
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 7200
#define DAYLIGHT_OFFSET_SEC 0
#define NTP_UPDATE_INTERVAL 3600000

// ============= PREFERENCES =============
#define PREF_NAMESPACE "wifi_config"

// ============= ТАЙМЕРИ =============
#define TIME_UPDATE_INTERVAL 1000
#define BUTTON_LONG_PRESS_TIME 1000
#define BUTTON_DEBOUNCE_DELAY 50

// ============= УПРАВЛІННЯ СВІТЛОДІОДАМИ =============
enum LedMode {
  LED_FORCE = 0,
  LED_OFF = 1
};

// ============= УПРАВЛІННЯ ЕКРАНАМИ =============
enum Screen {
  SCREEN_TIME = 0,
  SCREEN_NATURE = 1,
  SCREEN_SETTINGS = 2,
  SCREEN_COUNT
};

// ============= СТРУКТУРА ДАНИХ ПОГОДИ =============
struct WeatherData {
  String description = "No data";
  float temperature = 0.0;
  int humidity = 0;
  int pressure = 0;
  bool hasData = false;
  unsigned long lastUpdate = 0;
};

#endif // CONFIG_H