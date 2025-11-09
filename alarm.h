#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <NTPClient.h>
#include "config.h"

class AlarmManager {
private:
  int alarmHour;
  int alarmMinute;
  bool alarmEnabled;
  bool alarmTriggered;
  int lastAlarmDay;
  
  void playAlarmSound();

public:
  AlarmManager();
  
  void setupI2S();
  void checkAlarm(NTPClient& timeClient);
  
  // Getters
  int getHour() const { return alarmHour; }
  int getMinute() const { return alarmMinute; }
  bool isEnabled() const { return alarmEnabled; }
  bool isTriggered() const { return alarmTriggered; }
  
  // Setters
  void setTime(int hour, int minute);
  void setEnabled(bool enabled);
  void toggleEnabled();
  void resetTriggered();
};

#endif // ALARM_H