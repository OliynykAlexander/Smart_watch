#include "alarm.h"

AlarmManager::AlarmManager() 
  : alarmHour(9), alarmMinute(0), alarmEnabled(true), 
    alarmTriggered(false), lastAlarmDay(-1) {
}

void AlarmManager::setupI2S() {
  i2s_config_t config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);
}

void AlarmManager::playAlarmSound() {
  // 3 короткі сигнали
  for (int j = 0; j < 3; j++) {
    int samples = I2S_SAMPLE_RATE * 200 / 1000;  // 200мс на сигнал
    for (int i = 0; i < samples; i++) {
      int16_t sample = (int16_t)(sin(i * (2.0 * M_PI * I2S_BEEP_FREQ) / I2S_SAMPLE_RATE) * 15000);
      size_t bytes_written;
      i2s_write(I2S_NUM_0, (const char *)&sample, sizeof(sample), &bytes_written, portMAX_DELAY);
    }
    
    // Пауза 100мс між сигналами
    if (j < 2) {
      delay(100);
    }
  }
}

void AlarmManager::checkAlarm(NTPClient& timeClient) {
  if (!alarmEnabled) return;
 
  time_t epochTime = timeClient.getEpochTime();
  struct tm * timeinfo = gmtime(&epochTime);
 
  int currentHour = timeinfo->tm_hour;
  int currentMinute = timeinfo->tm_min;
  int currentDay = timeinfo->tm_yday;
 
  if (currentDay != lastAlarmDay) {
    alarmTriggered = false;
  }
 
  if (currentHour == alarmHour && currentMinute == alarmMinute && !alarmTriggered) {
    playAlarmSound();
    alarmTriggered = true;
    lastAlarmDay = currentDay;
  }
}

void AlarmManager::setTime(int hour, int minute) {
  if (hour >= 0 && hour <= 23) {
    alarmHour = hour;
  }
  if (minute >= 0 && minute <= 59) {
    alarmMinute = minute;
  }
  alarmTriggered = false;
}

void AlarmManager::setEnabled(bool enabled) {
  alarmEnabled = enabled;
  if (!alarmEnabled) {
    alarmTriggered = false;
  }
}

void AlarmManager::toggleEnabled() {
  alarmEnabled = !alarmEnabled;
  if (!alarmEnabled) {
    alarmTriggered = false;
  }
}

void AlarmManager::resetTriggered() {
  alarmTriggered = false;
}