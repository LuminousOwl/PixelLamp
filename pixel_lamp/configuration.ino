boolean settingsChanged = false;

struct {
  bool switchedOn;
  byte brightness;
  char displayMode[10];
  bool inWakeMode;
} lightSettings;

struct {
  bool switchedOn;
  byte brightness;
   int offsetMinutes;
  char transition[10];
  bool hourFormat24;
  byte state;
  bool changed;
  bool showIP;
} clockSettings;

struct daySettings {
  bool wakeUp;
  byte wakeUpHour;
  byte wakeUpMinute;
  byte wakeUpDuration;
};

daySettings wakeUpSettings[7];

Ticker writeSettings;

void settingsSetup() {  
  loadSettings();
  writeSettings.attach_ms(120000, handleSaveSettings);
}


void setLightValue(char* name, char* value) {
  Serial.println("set light value");
  if (strcmp(name, "light_switch") == 0) {
    lightSettings.switchedOn = strcmp("on",value) == 0;
  } else if (strcmp(name, "light_brightness") == 0) {
    lightSettings.brightness = atoi(value);
  } else if (strcmp(name, "light_mode") == 0) {
    strcpy(lightSettings.displayMode,value);
  } else {
    Serial.print("Light Setting Failed No Match:");
    Serial.print(name);
    Serial.print(":");
    Serial.println(value);
  }
}

void setClockValue(char* name, char* value) {
  clockSettings.changed = true;
  Serial.println("set clock value");
  Serial.println(name);
  Serial.println(value);
  if (strcmp(name, "clock_switch") == 0) {
    clockSettings.switchedOn = strcmp("on",value) == 0;
  } else if (strcmp(name, "clock_brightness") == 0) {
    clockSettings.brightness = atoi(value);
  } else if (strcmp(name, "clock_offset_minutes") == 0) {
    clockSettings.offsetMinutes = atoi(value);
  } else if (strcmp(name, "clock_transition") == 0) {
    strcpy(clockSettings.transition,value);
  } else if (strcmp(name, "clock_24_hour") == 0) {
    clockSettings.hourFormat24 = strcmp("on",value) == 0;
  } else if (strcmp(name, "clock_show_ip") == 0) {
    clockSettings.showIP = strcmp("on",value) == 0;
  } else {
    Serial.println("Clock Setting Failed");
  }
}

void setDayValue(char* name, char* value) {
  byte day;
  Serial.println("set day value");
  if (strstr(name, "0") != NULL) {
    day = 0;
  } else if (strstr(name, "1") != NULL) {
    day = 1;
  } else if (strstr(name, "2") != NULL) {
    day = 2;
  } else if (strstr(name, "3") != NULL) {
    day = 3;
  } else if (strstr(name, "4") != NULL) {
    day = 4;
  } else if (strstr(name, "5") != NULL) {
    day = 5;
  } else if (strstr(name, "6") != NULL) {
    day = 6;
  }

  if (strstr(name, "wake_up_time") != NULL) {
    if (strstr(name, "_h") != NULL) {
      wakeUpSettings[day].wakeUpHour = atoi(value);  
    } else if (strstr(name, "_m") != NULL) {
      wakeUpSettings[day].wakeUpMinute = atoi(value);
    } else {
      char *token;
      token = strtok(value, ":");
      wakeUpSettings[day].wakeUpHour = atoi(token);  
      token = strtok(NULL, ":");
      wakeUpSettings[day].wakeUpMinute = atoi(token);
    }
  } else if (strstr(name, "wake_up_duration") != NULL) {
    wakeUpSettings[day].wakeUpDuration = atoi(value);   
  } else if (strstr(name, "wake_up_") != NULL) {
    wakeUpSettings[day].wakeUp = strcmp("on",value) == 0;
  } else {
    Serial.println("Days Setting Failed");
  } 
}

void setValue(char* argName, char* argVal, bool markAsChanged) {
  if (markAsChanged) {
    settingsChanged = true;
  }
  
  if (strstr(argName, "wake_up_") != NULL) {
    setDayValue(argName, argVal);
  } else if (strstr(argName, "clock_") != NULL) {
    setClockValue(argName, argVal);
  } else if (strstr(argName, "light_") != NULL) {
    setLightValue(argName, argVal);
  } else {
    Serial.println("UNMATCHED REQUEST");
  }  
}

void loadDefaults() {
  Serial.println("load defaults");

  lightSettings.switchedOn = true;
  lightSettings.brightness = 127;
  strcpy(lightSettings.displayMode,"standard");
  lightSettings.inWakeMode = false;  

  clockSettings.switchedOn = true;
  clockSettings.brightness = 127;
  clockSettings.offsetMinutes = -240;
  strcpy(clockSettings.transition,"none");
  clockSettings.hourFormat24 = false;
  clockSettings.showIP = true;

  for (int d = 0; d < 7; d++) {
    wakeUpSettings[d].wakeUp = false;
    wakeUpSettings[d].wakeUpHour = 7;
    wakeUpSettings[d].wakeUpMinute = 0;
    wakeUpSettings[d].wakeUpDuration = 30;
  }
  
  saveSettings();
  Serial.println("load defaults end");
}

void loadSettings() {
  Serial.println("load settings");
  File settingsFile = SPIFFS.open("settings.txt", "r");

  if (!settingsFile) {
    Serial.println("load defaults");
    loadDefaults();
    return;
  }
  
  String name;
  String value;

  while(true) {
    name = settingsFile.readStringUntil('\n');
    if (name == "") { return; }
    value = settingsFile.readStringUntil('\n');
    if (value == "") { return; }
    Serial.print("name:");
    Serial.println(name);
    Serial.print("value:");
    Serial.println(value);
    
    char nameC[name.length() + 1];
    char valC[value.length() + 1];
    nameC[name.length()] = 0;
    valC[value.length()] = 0;
    
    name.toCharArray(nameC, name.length() + 1);
    value.toCharArray(valC, value.length() + 1);
    setValue(nameC, valC, false);    
  }
  clockSettings.switchedOn = true;
  settingsFile.close();
  
  Serial.println("load settings end");
}

void handleSaveSettings() {
  if (settingsChanged) {
    settingsChanged = false;
    saveSettings();
  }
}

void generateSettingsString(String* settingsString) {
  settingsString->concat("light_switch\n");
  settingsString->concat(lightSettings.switchedOn ? "on" : "off");
  settingsString->concat("\nlight_brightness\n");
  settingsString->concat(lightSettings.brightness);
  settingsString->concat("\nlight_mode\n");
  settingsString->concat(lightSettings.displayMode);
  
  settingsString->concat("\nclock_switch\n");
  settingsString->concat(clockSettings.switchedOn ? "on" : "off");
  settingsString->concat("\nclock_brightness\n");
  settingsString->concat(clockSettings.brightness);
  settingsString->concat("\nclock_offset_minutes\n");
  settingsString->concat(clockSettings.offsetMinutes);
  settingsString->concat("\nclock_transition\n");
  settingsString->concat(clockSettings.transition);
  settingsString->concat("\nclock_24_hour\n");
  settingsString->concat(clockSettings.hourFormat24 ? "on" : "off");
  settingsString->concat("\nclock_show_ip\n");
  settingsString->concat(clockSettings.showIP ? "on" : "off");
  
  for (int d = 0; d < 7; d++) {
    settingsString->concat("\nwake_up_time_");
    settingsString->concat(d);
    settingsString->concat("_h\n");
    settingsString->concat(wakeUpSettings[d].wakeUpHour);  

    settingsString->concat("\nwake_up_time_");
    settingsString->concat(d);
    settingsString->concat("_m\n");
    settingsString->concat(wakeUpSettings[d].wakeUpMinute);  
  
    settingsString->concat("\nwake_up_duration_");
    settingsString->concat(d);
    settingsString->concat("\n");    
    settingsString->concat(wakeUpSettings[d].wakeUpDuration);
    
    settingsString->concat("\nwake_up_");
    settingsString->concat(d);
    settingsString->concat("\n");    
    settingsString->concat(wakeUpSettings[d].wakeUp ? "on" : "off");  
  }
  settingsString->concat("\n");
  
  Serial.println(*settingsString);
}
void saveSettings() {
  Serial.println("start save settings");

  String fileSettingsString = "";

  generateSettingsString(&fileSettingsString);
  
  File settingsFile = SPIFFS.open("settings.txt", "w+");
  settingsFile.print(fileSettingsString);
  settingsFile.close();
  
  Serial.println("end save settings");
}


