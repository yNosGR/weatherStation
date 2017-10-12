#include <ESP8266WiFi.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>

WiFiManager wifiManager;
Adafruit_BMP085 bmp;
 
// replace with your channel’s thingspeak API privkey
String apiKey = "GetYourOwnDangKey";
const char* server = "api.thingspeak.com";

const int numReadings = 9;
int readIndex = 0;

float treadings[numReadings];
float treadIndex = 0;        
float ttotal = 0;            
float taverage = 0;          

float hreadings[numReadings];
float hreadIndex = 0;        
float htotal = 0;           
float haverage = 0;         

float preadings[numReadings]; 
float preadIndex = 0;         
float ptotal = 0;             
float paverage = 0;          

#define DHTPIN D4
#define DHTTYPE DHT11 
 
DHT dht(DHTPIN, DHTTYPE);

WiFiClient client;      
 
void setup()  {
  Serial.begin(115200); 

  // uncomment below to reset wifimanager settings
  /*
  Serial.println("clearing wifi settins");
  wifiManager.resetSettings();
   */
  
  Serial.println("Beginning setup");

  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 or BMP085 sensor at 0x77");
    while (1) {}   
  }
  
  
  
  delay(10);
  dht.begin();

  wifiManager.autoConnect("WeatherStation");

}
 
void loop(){

   
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float p = bmp.readPressure();
  
  // simple altitude compensation. 
  p = (p/100)+26 ; 
    
  if (client.connect(server,80)) {
    String postStr = apiKey;
    postStr +="&field1=";
    postStr += String(t);
    postStr +="&field2=";
    postStr += String(h);
    postStr +="&field3=";
    postStr += String(p);
    postStr += "\r\n\r\n";
     
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
     
    
  }
  client.stop();


  if (isnan(p) ) {
    Serial.println("Failed to read from BMP180! Sleeping for a moment.");
    delay(20000);
    return;
   }
   
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" degrees Celsius Humidity: ");
  Serial.print(h);
  Serial.print(" pressure in hPa: ");
  Serial.println(p);
  
 
  Serial.println("Waiting 20 secs");
  // thingspeak needs at least a 15 sec delay between updates
  // 20 seconds to be safe
  delay(20000);
}
