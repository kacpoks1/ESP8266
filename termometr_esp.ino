#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

float temp1, temp2;

const char* ssid = "SSID of your network";
const char* pass = "password of your network";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

void adjustTimeOffset() {
  timeClient.update();
  time_t rawTime = timeClient.getEpochTime();
  struct tm *timeInfo = gmtime(&rawTime);

  if ((timeInfo->tm_mon > 2 && timeInfo->tm_mon < 9) || 
      (timeInfo->tm_mon == 2 && timeInfo->tm_mday >= 31 - (5 * timeInfo->tm_wday) / 4) || 
      (timeInfo->tm_mon == 9 && timeInfo->tm_mday < 31 - (5 * timeInfo->tm_wday) / 4)) {
    timeClient.setTimeOffset(7200); // Czas letni (UTC+2)
  } else {
    timeClient.setTimeOffset(3600); // Czas zimowy (UTC+1)
  }
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("Client connected");
  }
}

unsigned long previousMillis = 0;
const long interval = 5000; // 5 seconds
bool displayTime = false;

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("\u0141\u0105czenie z WIFI...");
  }
  Serial.println("Po\u0142\u0105czono z WiFi");
  Serial.print(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected!");
  delay(2000);

  sensors.begin();
  timeClient.begin();
  adjustTimeOffset();

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "Temperature Monitor");
  });

  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  sensors.requestTemperatures();
  temp1 = sensors.getTempCByIndex(0);
  temp2 = sensors.getTempCByIndex(1);
  ws.textAll("{\"temp\":" + String(temp1) + ", \"temp1\":" + String(temp2) + "}");

  timeClient.update();
  adjustTimeOffset();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    displayTime = !displayTime;

    if (displayTime) {
      time_t rawTime = timeClient.getEpochTime();
      struct tm *timeInfo = localtime(&rawTime);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Time: ");
      lcd.print(timeClient.getFormattedTime());
      lcd.setCursor(0, 1);
      lcd.print("Date: ");
      lcd.print(timeInfo->tm_mday);
      lcd.print("-");
      lcd.print(timeInfo->tm_mon + 1);
      lcd.print("-");
      lcd.print(timeInfo->tm_year + 1900);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Out: ");
      lcd.print(temp1);
      lcd.print((char)223);
      lcd.print("C");
      lcd.setCursor(0, 1);
      lcd.print("In: ");
      lcd.print(temp2);
      lcd.print((char)223);
      lcd.print("C");
    }
  }
  delay(1000);
}
