#define BLYNK_TEMPLATE_ID "TMPL4Unf0Yv4S"
#define BLYNK_TEMPLATE_NAME "Pogodynka"
#define BLYNK_AUTH_TOKEN "mTGH2nCrdA4a5hrsMmM3JeO5owcEyXvz"

#define BLYNK_PRINT Serial // Comment this out to disable prints and save space
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
BlynkTimer timer;

int lcdColumns = 16;
int lcdRows = 2;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

float temp, temp1;  
 
    
    // You should get Auth Token in the Blynk App.
    // Go to the Project Settings (nut icon).
    
    char auth[] = "mTGH2nCrdA4a5hrsMmM3JeO5owcEyXvz";
    char ssid[] = "Orange_Swiatlowod_9750_EXT";
    char pass[] = "SSQQ635UUMTL";

   
    void setup()
    {
     Serial.begin(115200);
     sensors.begin();
     Blynk.begin(auth, ssid, pass);
     timer.setInterval(3000L, sendTemps);
     lcd.init();
     lcd.clear();         
     lcd.backlight();
         
    }
  
void sendTemps()
    {
 sensors.requestTemperatures(); // Polls the sensors
 temp = sensors.getTempCByIndex(0); // Gets first temperature reading
 temp1 = sensors.getTempCByIndex(1); // Gets second temperature reading
 Serial.println(temp);
 Serial.println(temp1);
 Blynk.virtualWrite(V0, temp); // Sends temperature values to Blynk
 Blynk.virtualWrite(V1, temp1); 
  
    }

void loop()
    {
       if (Blynk.connected()){ Blynk.run();}
       timer.run();

        if (temp < 0 );{
        Blynk.logEvent("zimno",String ("Temperatura poniżej 0, Tº: ") + temp);
       }

       
       
        lcd.setCursor(0, 0);
        lcd.print("Dwor: ");
        lcd.setCursor(6,0);
        lcd.print(temp);
        lcd.print((char)223);
        lcd.print("C");
        lcd.setCursor(0,1);
        lcd.print("Dom: ");
        lcd.setCursor(6,1);
        lcd.print(temp1);
        lcd.print((char)223);
        lcd.print("C");
      
        
    }
