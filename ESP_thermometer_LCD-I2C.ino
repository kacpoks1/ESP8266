#define BLYNK_TEMPLATE_ID "Your ID" //Your ID from Blynk
#define BLYNK_TEMPLATE_NAME "Name_of_your_device"   //Name for your device
#define BLYNK_AUTH_TOKEN "Your token"   //Your token from Blynk

#define BLYNK_PRINT Serial 
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

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); // Including LCD display with I2C (on esp8266 SLC pin to D1 and SDA pin to D2)

float temp, temp1;  
 
    
    
    
    char auth[] = "Your token";  //Your token from Blynk
    char ssid[] = "Your SSID";   //Name of your network
    char pass[] = "Your password";  //Password to your network

   
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
 sensors.requestTemperatures(); 
 temp = sensors.getTempCByIndex(0); 
 temp1 = sensors.getTempCByIndex(1); 
 Serial.println(temp);
 Serial.println(temp1);
 Blynk.virtualWrite(V0, temp);    //Virtual pins to set up on Blynk (V0)
 Blynk.virtualWrite(V1, temp1);   //Virtual pins to set up on Blynk (V1)
  
    }

void loop()
    {
       if (Blynk.connected()){ Blynk.run();}
       timer.run();
       
        lcd.setCursor(0, 0);
        lcd.print("Out: "); //Displaying temperature from outside in Celsius
        lcd.print(temp);
        lcd.print((char)223);
        lcd.print("C");
        lcd.setCursor(0,1);
        lcd.print("In: ");  //Displaying temperature from inside in Celsius
        lcd.print(temp1);
        lcd.print((char)223);
        lcd.print("C");
          
    }
