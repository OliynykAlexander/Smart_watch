#include "wifi_manager.h"

WiFiManager::WiFiManager(Storage* stor, AlarmManager* alarm, WeatherManager* weather,
                         Screen* screen, bool* needUpdate)
  : server(WEB_SERVER_PORT), storage(stor), alarmManager(alarm), 
    weatherManager(weather), currentScreen(screen), needUpdateSetScreen(needUpdate) {
}

void WiFiManager::begin() {
  server.on("/", [this]() { handleRoot(); });
  server.on("/connect", [this]() { handleConnect(); });
  server.on("/status", [this]() { handleStatus(); });
  server.on("/alarm", [this]() { handleAlarm(); });
  server.on("/weather/update", [this]() { handleWeatherUpdate(); });
  server.on("/weather/apikey", [this]() { handleWeatherApiKey(); });
  server.onNotFound([this]() { handleNotFound(); });
  server.begin();
}

void WiFiManager::handleClient() {
  server.handleClient();
}

bool WiFiManager::connectToWiFi(const String& ssid, const String& password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
  }
  
  return (WiFi.status() == WL_CONNECTED);
}

void WiFiManager::startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
}

bool WiFiManager::isConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

void WiFiManager::handleRoot() {
  const char html_template[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset='utf-8'>
  <meta name='viewport' content='width=device-width,initial-scale=1'>
  <title>ESP Terminal Control</title>
  <style>
    * {margin:0;padding:0;box-sizing:border-box}
    body {
      font-family:'Courier New',monospace;
      background:#0a0e14;
      color:#00ff41;
      min-height:100vh;
      padding:20px;
    }
    .container {
      max-width:800px;
      margin:0 auto;
      background:#1a1f2e;
      border:2px solid #00ff41;
      border-radius:5px;
      padding:20px;
      box-shadow:0 0 20px rgba(0,255,65,.3);
    }
    h1 {
      text-align:center;
      margin-bottom:20px;
      text-shadow:0 0 10px #00ff41;
    }
    .section {
      margin:20px 0;
      padding:15px;
      border:1px solid #00ff41;
      border-radius:3px;
    }
    .section h2 {margin-bottom:10px;color:#00ccff}
    button {
      background:#00ff41;
      color:#0a0e14;
      border:none;
      padding:10px 20px;
      margin:5px;
      cursor:pointer;
      font-family:'Courier New',monospace;
      font-weight:bold;
      border-radius:3px;
    }
    button:hover {background:#00ccff}
    input {
      background:#0a0e14;
      border:1px solid #00ff41;
      color:#00ff41;
      padding:8px;
      margin:5px;
      font-family:'Courier New',monospace;
    }
    #apiKey {
      width: calc(100% - 10px);
    }
    .status {
      padding:10px;
      margin:10px 0;
      background:#0a0e14;
      border-left:3px solid #00ff41;
    }
    .info {color:#00ccff}
    .warning {color:#ffaa00}
    .error {color:#ff4444}
  </style>
</head>
<body>
  <div class='container'>
    <h1>ESP Terminal</h1>
    
    <div class='section'>
      <h2>📡 WiFi Configuration</h2>
      <input type='text' id='ssid' placeholder='SSID'>
      <input type='password' id='password' placeholder='Password'>
      <button onclick='connectWiFi()'>Connect</button>
      <div class='status' id='wifiStatus'></div>
    </div>
    
    <div class='section'>
      <h2>🔑 Weather API Key</h2>
      <input type='text' id='apiKey' placeholder='OpenWeatherMap API Key'>
      <button onclick='setApiKey()'>Save API Key</button>
      <div class='status' id='apiKeyStatus'></div>
    </div>
    
    <div class='section'>
      <h2>📊 System Status</h2>
      <div class='status' id='systemStatus'></div>
    </div>
    
    <div class='section'>
      <h2>⏰ Alarm Settings</h2>
      <input type='number' id='alarmHour' min='0' max='23' placeholder='Hour' value='9'>
      <input type='number' id='alarmMinute' min='0' max='59' placeholder='Minute' value='0'>
      <button onclick='setAlarm()'>Set Alarm</button>
      <button onclick='toggleAlarm()'>Toggle On/Off</button>
      <div class='status' id='alarmStatus'></div>
    </div>
    
    <div class='section'>
      <h2>🌤️ Weather Data</h2>
      <button onclick='updateWeather()'>Update Weather</button>
      <div class='status' id='weatherStatus'></div>
    </div>
  </div>
  
  <script>
    const api = (url, opts) => fetch(url, opts).then(r => opts?.json !== false ? r.json() : r.text());
    
    function connectWiFi() {
      const ssid = document.getElementById('ssid').value;
      const password = document.getElementById('password').value;
      const params = new URLSearchParams({ssid, password});
      
      api('/connect', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: params,
        json: false
      })
      .then(data => {
        document.getElementById('wifiStatus').innerHTML = `<span class="info">✓ ${data}</span>`;
      })
      .catch(e => {
        document.getElementById('wifiStatus').innerHTML = `<span class="error">✗ ${e}</span>`;
      });
    }
    
    function setApiKey() {
      const apiKey = document.getElementById('apiKey').value;
      
      if (!apiKey) {
        document.getElementById('apiKeyStatus').innerHTML = 
          '<span class="error">✗ Please enter API key</span>';
        return;
      }
      
      api('/weather/apikey', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({apiKey})
      })
      .then(data => {
        document.getElementById('apiKeyStatus').innerHTML = 
          `<span class="info">✓ API Key saved: ${data.apiKey}</span>`;
        document.getElementById('apiKey').value = '';
      })
      .catch(e => {
        document.getElementById('apiKeyStatus').innerHTML = 
          `<span class="error">✗ Failed: ${e}</span>`;
      });
    }
    
    function getStatus() {
      api('/status')
      .then(data => {
        const html = [
          `<span class="info">IP: ${data.ip}</span>`,
          `Screen: ${data.screen}`,
          `Uptime: ${(data.uptime / 1000).toFixed(0)}s`,
          `Alarm: ${data.alarm.hour}:${data.alarm.minute} (${data.alarm.enabled ? 'ON' : 'OFF'})`
        ].join('<br>');
        document.getElementById('systemStatus').innerHTML = html;
      });
      
      api('/weather/apikey')
      .then(data => {
        if (data.hasKey) {
          document.getElementById('apiKeyStatus').innerHTML = 
            `<span class="info">✓ API Key configured: ${data.apiKey}</span>`;
        }
      });
    }
    
    function setAlarm() {
      const hour = parseInt(document.getElementById('alarmHour').value);
      const minute = parseInt(document.getElementById('alarmMinute').value);
      
      api('/alarm', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({hour, minute})
      })
      .then(data => {
        document.getElementById('alarmStatus').innerHTML = 
          `<span class="info">✓ Alarm set to ${data.hour}:${data.minute}</span>`;
      });
    }
    
    function toggleAlarm() {
      api('/alarm', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({toggle: true})
      })
      .then(data => {
        document.getElementById('alarmStatus').innerHTML = 
          `<span class="info">✓ Alarm ${data.enabled ? 'enabled' : 'disabled'}</span>`;
      });
    }
    
    function updateWeather() {
      api('/weather/update')
      .then(data => {
        const html = [
          '<span class="info">✓ Updated!</span>',
          `Description: ${data.description}`,
          `Temperature: ${data.temperature}°C`,
          `Humidity: ${data.humidity}%`,
          `Pressure: ${data.pressure} hPa`
        ].join('<br>');
        document.getElementById('weatherStatus').innerHTML = html;
      })
      .catch(e => {
        document.getElementById('weatherStatus').innerHTML = 
          `<span class="error">✗ Update failed: ${e}</span>`;
      });
    }
    
    setInterval(getStatus, 5000);
    getStatus();
  </script>
</body>
</html>
)rawliteral";
  
  server.send_P(200, "text/html; charset=utf-8", html_template);
}

void WiFiManager::handleConnect() {
  if (server.method() == HTTP_POST) {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    
    if (ssid.length() > 0 && password.length() > 0) {
      storage->saveWiFiCredentials(ssid, password);
      
      server.send(200, "text/plain; charset=utf-8", "Connecting to " + ssid + "...");
      
      delay(1000);
      connectToWiFi(ssid, password);
      
      if (isConnected()) {
        configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
      }
    } else {
      server.send(400, "text/plain; charset=utf-8", "Invalid credentials");
    }
  }
}

void WiFiManager::handleStatus() {
  StaticJsonDocument<512> doc;
  
  doc["ip"] = WiFi.localIP().toString();
  doc["connected"] = isConnected();
  doc["screen"] = *currentScreen;
  doc["uptime"] = millis();
  
  JsonObject alarm = doc.createNestedObject("alarm");
  alarm["hour"] = alarmManager->getHour();
  alarm["minute"] = alarmManager->getMinute();
  alarm["enabled"] = alarmManager->isEnabled();
  alarm["triggered"] = alarmManager->isTriggered();
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void WiFiManager::handleAlarm() {
  if (server.method() == HTTP_POST) {
    String body = server.arg("plain");
   
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, body);
   
    if (!error) {
      if (doc.containsKey("hour") && doc.containsKey("minute")) {
        alarmManager->setTime(doc["hour"], doc["minute"]);
      }
      
      if (doc.containsKey("enabled")) {
        alarmManager->setEnabled(doc["enabled"]);
      }
      
      if (doc.containsKey("toggle")) {
        alarmManager->toggleEnabled();
      }

      storage->saveAlarmSettings(alarmManager->getHour(), 
                                 alarmManager->getMinute(), 
                                 alarmManager->isEnabled());
     
      if (*currentScreen == SCREEN_SETTINGS) {
        *needUpdateSetScreen = true;
      }
     
      StaticJsonDocument<512> response;
      response["hour"] = alarmManager->getHour();
      response["minute"] = alarmManager->getMinute();
      response["enabled"] = alarmManager->isEnabled();
      response["triggered"] = alarmManager->isTriggered();
      
      String responseStr;
      serializeJson(response, responseStr);
      server.send(200, "application/json", responseStr);
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
    }
  } else {
    StaticJsonDocument<512> doc;
    doc["hour"] = alarmManager->getHour();
    doc["minute"] = alarmManager->getMinute();
    doc["enabled"] = alarmManager->isEnabled();
    doc["triggered"] = alarmManager->isTriggered();
   
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }
}

void WiFiManager::handleWeatherUpdate() {
  weatherManager->fetchWeatherData();
  
  StaticJsonDocument<512> doc;
  doc["description"] = weatherManager->getDescription();
  doc["temperature"] = weatherManager->getTemperature();
  doc["humidity"] = weatherManager->getHumidity();
  doc["pressure"] = weatherManager->getPressure();
  doc["status"] = "updated";
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void WiFiManager::handleWeatherApiKey() {
  if (server.method() == HTTP_POST) {
    String body = server.arg("plain");
    
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (!error && doc.containsKey("apiKey")) {
      String apiKey = doc["apiKey"].as<String>();
      weatherManager->setApiKey(apiKey);
      storage->saveWeatherApiKey(apiKey);
      
      StaticJsonDocument<256> response;
      response["status"] = "success";
      response["apiKey"] = apiKey.substring(0, 8) + "...";
      
      String responseStr;
      serializeJson(response, responseStr);
      server.send(200, "application/json", responseStr);
      
      if (apiKey.length() > 0) {
        weatherManager->fetchWeatherData();
      }
    } else {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid data\"}");
    }
  } else {
    StaticJsonDocument<256> doc;
    doc["hasKey"] = weatherManager->hasApiKey();
    doc["apiKey"] = weatherManager->hasApiKey() ? 
                    weatherManager->getApiKey().substring(0, 8) + "..." : "";
    
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  }
}

void WiFiManager::handleNotFound() {
  server.send(404, "text/plain", "404 - Page not found");
}