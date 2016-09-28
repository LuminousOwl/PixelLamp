#define MATRIX_IP_STATE 0
#define MATRIX_TIME_STATE 1

byte matrixMinutes = 255;
int8_t matrixCursorX = 16;

// 3x9 bitmaps digits 0 - 9
const unsigned char bitmap0 [] PROGMEM = {
B11100000,
B10100000,
B10100000,
B10100000,
B10100000,
B10100000,
B10100000,
B10100000,
B11100000
};
const unsigned char bitmap1 [] PROGMEM = {
B01000000,
B01000000,
B01000000,
B01000000,
B01000000,
B01000000,
B01000000,
B01000000,
B01000000
};
const unsigned char bitmap2 [] PROGMEM = {
B11100000,
B00100000,
B00100000,
B00100000,
B11100000,
B10000000,
B10000000,
B10000000,
B11100000
};
const unsigned char bitmap3 [] PROGMEM = {
B11100000,
B00100000,
B00100000,
B00100000,
B11100000,
B00100000,
B00100000,
B00100000,
B11100000
};
const unsigned char bitmap4 [] PROGMEM = {
B10100000,
B10100000,
B10100000,
B10100000,
B11100000,
B00100000,
B00100000,
B00100000,
B00100000
};
const unsigned char bitmap5 [] PROGMEM = {
B11100000,
B10000000,
B10000000,
B10000000,
B11100000,
B00100000,
B00100000,
B00100000,
B11100000
};
const unsigned char bitmap6 [] PROGMEM = {
B11100000,
B10000000,
B10000000,
B10000000,
B11100000,
B10100000,
B10100000,
B10100000,
B11100000
};
const unsigned char bitmap7 [] PROGMEM = {
B11100000,
B00100000,
B00100000,
B00100000,
B00100000,
B00100000,
B00100000,
B00100000,
B00100000
};
const unsigned char bitmap8 [] PROGMEM = {
B11100000,
B10100000,
B10100000,
B10100000,
B11100000,
B10100000,
B10100000,
B10100000,
B11100000
};

const unsigned char bitmap9 [] PROGMEM = {
B11100000,
B10100000,
B10100000,
B10100000,
B11100000,
B00100000,
B00100000,
B00100000,
B00100000
};

const unsigned char bitmapC [] PROGMEM = {
B00000000,
B00000000,
B01000000,
B00000000,
B00000000,
B00000000,
B01000000,
B00000000,
B00000000
};

const unsigned char* bitmaps[] = { bitmap0, bitmap1, bitmap2, bitmap3, bitmap4, bitmap5, bitmap6, bitmap7, bitmap8, bitmap9 };

Ticker matrixRefresh;

Adafruit_IS31FL3731 matrix = Adafruit_IS31FL3731();

void matrixSetup() {
  clockSettings.state = MATRIX_IP_STATE; 
  
  if (!matrix.begin()) {
    Serial.println("IS31 not found");
    while (1);
  }
  matrix.setTextWrap(false);
  matrix.setTextSize(1);

  matrixRefresh.attach_ms(60, handleMatrix);
}

// Draw a digit on the matrix, 
// digit: digit to draw
// startX: 1st column to draw on
void drawDigit(byte digit, int startX) {
  matrix.drawBitmap(startX,0,bitmaps[digit], 3, 9, clockSettings.brightness);
}

// Draw a ':' character on the matrix
void drawColon(byte x) {
  matrix.drawBitmap(x,0,bitmapC, 3, 9, clockSettings.brightness);
}

void handleMatrix() {
  // Show the ip address for 90 seconds
  // or until a request has been received;
  if (!clockSettings.showIP || timeElapsed > 90000 || firstRequestReceived) {
    matrixRefresh.detach();
    matrixRefresh.attach_ms(500, handleMatrix);
    clockSettings.state = MATRIX_TIME_STATE;
  }

  if (clockSettings.state == MATRIX_IP_STATE) {
    matrix.setTextColor(127);
    
    if (matrixCursorX < -96) {
      matrixCursorX = 16;
    }
    matrixCursorX -= 1;
    matrix.clear();
    matrix.setCursor(matrixCursorX,0);
    matrix.print(WiFi.localIP());      
  } else if (clockSettings.state == MATRIX_TIME_STATE) {
    timeClient.setTimeOffset(clockSettings.offsetMinutes * 60);
    if (matrixMinutes == timeClient.getMinutes() && !clockSettings.changed) { return; }
    clockSettings.changed = false;
    matrix.clear();
    if (clockSettings.switchedOn == false) { return; }

    matrixMinutes = timeClient.getMinutes();
    byte matrixHours = timeClient.getHours();
    byte column = 0;
    Serial.println(matrixHours);

    if (matrixHours > 12 && !clockSettings.hourFormat24) {
      matrixHours -= 12;
    } else if (matrixHours == 0 && !clockSettings.hourFormat24) {
      matrixHours = 12;
    }

    // Ones digits and tens digits for hours and minutes
    byte hoursOne = matrixHours % 10;
    byte hoursTen = (matrixHours - hoursOne) / 10;
    byte minutesOne = matrixMinutes % 10;
    byte minutesTen = (matrixMinutes - minutesOne) / 10;
       
    // If 24 hour output the numbers without ':' 
    // character since it can't always fit
    if (clockSettings.hourFormat24) {
      drawDigit(hoursTen, 0); 
      drawDigit(hoursOne, 4); 
      drawDigit(minutesTen, 8);
      drawDigit(minutesOne, 12);
      return;     
    }

    // If hours 10 or more, draw the first '1' offset 
    // so it only take two columns and we can fit the ':' 
    if (matrixHours >= 10) {
      drawDigit(1, -1);
      drawDigit(hoursOne, 2);
      drawColon(5);
      drawDigit(minutesTen, 8);
      drawDigit(minutesOne, 12);
    } else {
      drawDigit(hoursOne, 2);
      drawColon(5);
      drawDigit(minutesTen, 8);
      drawDigit(minutesOne, 12);
    }
  }
}


