#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

float temp1, temp2;
unsigned long previousMillis = 0;
const long interval = 5000; // 5 seconds
bool showTemp = false;
bool timeInitialized = false;

const char* ssid = "Your Network SSID";
const char* password = "Your Network Password"; 

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // 60s update interval

void displayMessage(String line1, String line2 = "") {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (line2 != "") {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
  delay(2000);
}

void adjustTimeOffset() {
  time_t now = timeClient.getEpochTime();
  if (now < 100000) return;
  
  struct tm *timeinfo = localtime(&now);
  int year = timeinfo->tm_year + 1900;
  int month = timeinfo->tm_mon + 1;
  int day = timeinfo->tm_mday;
  int hour = timeinfo->tm_hour;
  
  if (month > 3 && month < 10) {
    timeClient.setTimeOffset(7200); // CEST (UTC+2)
  } else if (month == 3) {
    int lastSunday = (31 - (5 * year / 4 + 4) % 7);
    if (day > lastSunday || (day == lastSunday && hour >= 2)) {
      timeClient.setTimeOffset(7200);
    }
  } else if (month == 10) {
    int lastSunday = (31 - (5 * year / 4 + 1) % 7);
    if (day < lastSunday || (day == lastSunday && hour < 3)) {
      timeClient.setTimeOffset(7200);
    }
  }
}

void displayDateTime() {
  if (!timeInitialized) {
    displayMessage("Time not synced!", "Using local time");
    return;
  }
  
  timeClient.update();
  adjustTimeOffset();
  
  String timeStr = timeClient.getFormattedTime();
  unsigned long epochTime = timeClient.getEpochTime();
  
  time_t rawtime = epochTime;
  struct tm *timeinfo;
  timeinfo = localtime(&rawtime);
  
  char dateStr[11];
  sprintf(dateStr, "%02d-%02d-%04d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Date:");
  lcd.print(dateStr);
  lcd.setCursor(0, 1);
  lcd.print("Time:");
  lcd.print(timeStr.substring(0, 5)); // Hours and minutes
}

void displayTemperature() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Out:");
  lcd.print(temp1);
  lcd.print((char)223);
  lcd.print("C");
  
  lcd.setCursor(0, 1);
  lcd.print("In:");
  lcd.print(temp2);
  lcd.print((char)223);
  lcd.print("C");
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  
  displayMessage("Connecting to WiFi...", ssid);
  
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    attempts++;
    if (attempts % 4 == 0) {
      displayMessage("Connecting...", "Attempt " + String(attempts/2) + "/10");
    }
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    displayMessage("Connection failed", "Check settings");
    while(1) delay(1000);
  }
  
  String ip = WiFi.localIP().toString();
  displayMessage("WiFi connected", "IP: " + ip);
  delay(2000);

  sensors.begin();
  displayMessage("Initializing", "sensors...");
  
  timeClient.begin();
  displayMessage("Getting time", "from NTP...");
  
  int timeAttempts = 0;
  while (timeAttempts < 10) {
    if (timeClient.update() && timeClient.getEpochTime() > 100000) {
      timeInitialized = true;
      adjustTimeOffset();
      displayMessage("Time synced!", "with NTP server");
      break;
    }
    delay(1000);
    timeAttempts++;
    if (timeAttempts % 3 == 0) {
      displayMessage("Attempt " + String(timeAttempts) + "/10", "Syncing time...");
    }
  }
  
  if (!timeInitialized) {
    displayMessage("Sync failed", "Using local time");
  }
}

void loop() {
  unsigned long currentMillis = millis();
  
  sensors.requestTemperatures();
  temp1 = sensors.getTempCByIndex(0);
  temp2 = sensors.getTempCByIndex(1);
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    if (showTemp) {
      displayTemperature();
    } else {
      displayDateTime();
    }
    
    showTemp = !showTemp;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    timeInitialized = false;
    WiFi.reconnect();
    delay(1000);
  }
  
  if (timeInitialized && currentMillis % 60000 == 0) {
    if (!timeClient.update()) {
      timeInitialized = false;
      displayMessage("NTP update", "failed!");
      delay(2000);
    }
  }
  
  delay(100);
}
