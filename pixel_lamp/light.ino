#define NEOPIXEL_PIN 12 
#define PIXEL_COUNT 42 // Number of LEDs in strip

uint16_t rainbowCycleIndex = 0;
uint16_t rainbowIndex = 0;

Adafruit_DotStar strip = Adafruit_DotStar(
  PIXEL_COUNT, 13, 14, DOTSTAR_BRG);

Ticker lightRefresh;
Ticker wakeUpRefresh;

void lightSetup() {
  strip.begin();
  strip.show();
  lightRefresh.attach_ms(30, handleLight);
  wakeUpRefresh.attach_ms(1000, handleWakeUp);
}

// Rainbow with multiple colors at once
void rainbowCycle() {
  uint16_t i, j;

  rainbowCycleIndex += 1;
  if (rainbowCycleIndex >= (256 * 5)) {
    rainbowCycleIndex = 0;
  }
      
  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + rainbowCycleIndex) & 255));
  } 
}

// Rainbow on color at a time
void rainbow() {
  uint16_t i;
  
  rainbowIndex += 1;
  if (rainbowIndex >= 256) {
    rainbowIndex = 0;
  }

  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i+rainbowIndex) & 255));
  } 
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color((255 - WheelPos * 3), 0, (WheelPos * 3));
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, (WheelPos * 3), (255 - WheelPos * 3));
  }
  WheelPos -= 170;
  return strip.Color((WheelPos * 3), (255 - WheelPos * 3), 0);
}

// Called to update the lighting
void handleLight() {
  if (lightSettings.switchedOn && lightSettings.brightness > 0) {
    strip.setBrightness(lightSettings.brightness);
    
    if ( strstr(lightSettings.displayMode, "standard") != NULL) {
      // Default white
      for (int pixelIndex = 0; pixelIndex < PIXEL_COUNT; pixelIndex++) {
        strip.setPixelColor(pixelIndex, 255, 255, 255);
      }
    } else if ( strstr(lightSettings.displayMode, "warm") != NULL) {
      // Warmer white
      for (int pixelIndex = 0; pixelIndex < PIXEL_COUNT; pixelIndex++) {
        strip.setPixelColor(pixelIndex, 255, 255, 100);
      }
    } else if ( strstr(lightSettings.displayMode, "cool") != NULL) {
      // Cooler White
      for (int pixelIndex = 0; pixelIndex < PIXEL_COUNT; pixelIndex++) {
        strip.setPixelColor(pixelIndex, 150, 150, 255);
      }
    } else if ( strstr(lightSettings.displayMode, "rainbow1") != NULL) {
      // Rainbow multiple colors at once
      rainbowCycle();
    } else if ( strstr(lightSettings.displayMode, "rainbow2") != NULL) {
      // Rainbow one color at a time
      rainbow();
    }
  } else {
    // No light at all
    for (int pixelIndex = 0; pixelIndex < PIXEL_COUNT; pixelIndex++) {
      strip.setPixelColor(pixelIndex,0,0,0);
    }    
  }
  strip.show();
}

// Handle light values for wake up mode
void handleWakeUp() {
   // Make sure the time offset is set
   timeClient.setTimeOffset(clockSettings.offsetMinutes * 60);

   // Get the settings for today
   daySettings settings = wakeUpSettings[timeClient.getDay()];

   // If no wake up for today return
   if (!settings.wakeUp) { return; }

   // Not currently in wake up mode?
   if (!lightSettings.inWakeMode) {
      // If the hour or minute don't match the wake up time leave      
      if (timeClient.getHours() != settings.wakeUpHour) {
        return;
      }
      if (timeClient.getMinutes() != settings.wakeUpMinute) {
        return;
      }

      // Hour an minute matched, initialize wake up
      lightSettings.brightness = 2;
      lightSettings.inWakeMode = true;
      lightSettings.switchedOn = true;  
      timeElapsed = 0;    
   } 

   // We are in wake up mode, see if wake up time is done
   if (timeElapsed > settings.wakeUpDuration * 60 * 1000) {
     lightSettings.inWakeMode = false;
     lightSettings.brightness = 255;
     return;
   }

   // Calculate the brightness based on the wake up duration
   int newBrightness = (timeElapsed / (settings.wakeUpDuration * 60 * 1000.0)) * 255.0;
   
   if (newBrightness > lightSettings.brightness) {
     Serial.print("new brightness:");
     Serial.println(newBrightness);
     lightSettings.brightness = newBrightness;    
   }
}


