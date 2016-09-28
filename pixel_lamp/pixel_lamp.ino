#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include <SPI.h>  
#include <FS.h>
#include <NTPClient.h>
#include <elapsedMillis.h>
#include <Adafruit_IS31FL3731.h>
#include <Adafruit_GFX.h>
#include <Adafruit_DotStar.h>

//////////// CONFIGURE FOR YOUR WIFI ////////////
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
////////////////////////////////////////////////

elapsedMillis timeElapsed;

WiFiUDP ntpUDP; 
NTPClient timeClient(ntpUDP, "time.nist.gov", 0, 300000);
ESP8266WebServer server(80);

char* html;
int htmlSize;
boolean firstRequestReceived = false;

void setup() {
  //wdt_disable();
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  delay(10);
    
  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
   
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
   
  SPIFFS.begin();
  File f = SPIFFS.open("/light.html", "r");
  htmlSize = f.size();
  html = (char*)malloc(htmlSize);
  f.readBytes(&html[0],htmlSize);

  server.on("/", [](){
    firstRequestReceived = true;
    server.send(200, "text/html", html);
  });

  server.on("/reset_settings", []() {
    firstRequestReceived = true;
    loadDefaults();
    saveSettings();
    server.send(200, "text", "ok"); 
  });
  
  server.on("/set", [](){
    firstRequestReceived = true;
    Serial.println("set");
    Serial.println(server.arg(0)); 
    Serial.println(server.argName(0)); 
    
    char argName[server.argName(0).length() + 1];
    char argVal[server.arg(0).length() + 1];
    
    server.argName(0).toCharArray(argName, server.argName(0).length() + 1);
    server.arg(0).toCharArray(argVal, server.arg(0).length() + 1);
    
    setValue(argName, argVal, true);   
    
    server.send(200, "text/html", "ok");
  });

  server.on("/settings", []() { 
    firstRequestReceived = true;
    String returnString = "";
    generateSettingsString(&returnString);
    Serial.println("RETURN STRING START");
    Serial.println(returnString);  
    Serial.println("RETURN STRING END");
    server.send(200, "text", returnString);
  });

  server.on("/apple-touch-icon-180x180.png", []() {
    Serial.println("apple icon request");
    File file = SPIFFS.open("/apple-touch-icon-180x180.png","r");
    server.streamFile(file, "image/png");
  });
  
  server.on("/icon-hires.png", []() {
    Serial.println("android icon request");
    File file = SPIFFS.open("/icon-hires.png","r");
    server.streamFile(file, "image/png");
  });

  
  timeClient.begin();
  timeElapsed = 0;

  settingsSetup();
  matrixSetup();
  lightSetup();
}

void loop() {  
  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    WiFi.disconnect();
    WiFi.begin(ssid, password);
    return;
  }
  
  server.handleClient();
  wdt_disable();
  timeClient.update();
  wdt_enable(WDTO_4S);  
  //loopWakeUp();
  //delay(10);
}












