#include <DHT.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;
 
// replace with your channel’s thingspeak API key and your SSID and password
String apiKey = "YOURAPIKEY";
const char* ssid = "YOURSSID";
const char* password = "YOURPASSWPHRASE";
const char* server = "api.thingspeak.com";

const int numReadings = 10;
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
  
  for ( int thisReading = 0 ; thisReading < numReadings; thisReading++) {
    treadings[thisReading] = 0;
    hreadings[thisReading] = 0;
    preadings[thisReading] = 0;
  }

  if (!bmp.begin()) {
    Serial.println("Could not find BMP180 or BMP085 sensor at 0x77");
    while (1) {}   
  }
  
  
  Serial.begin(115200);
  delay(10);
  dht.begin();
   
  WiFi.begin(ssid, password);
   
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
   
  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
 
}
 
void loop(){

   
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float p = bmp.readPressure();
  
  // simple altitude compensation. 
  p = (p/100)+26 ; 

  htotal = htotal - hreadings[readIndex];
  hreadings[readIndex] = h;
  htotal = htotal + hreadings[readIndex];
  haverage = htotal / numReadings;
  
  ttotal = ttotal - treadings[readIndex];
  treadings[readIndex] = t;
  ttotal = ttotal + treadings[readIndex];
  taverage = ttotal / numReadings;

  ptotal = ptotal - preadings[readIndex];
  preadings[readIndex] = p;
  ptotal = ptotal + preadings[readIndex];
  paverage = ptotal / numReadings;
  
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) {
    readIndex = 1;

    Serial.print("Average Temperature: ");
    Serial.print(taverage);
    Serial.print(" Average humidity: ");
    Serial.print(haverage);
    Serial.print(" pressure in hPa: ");
    Serial.println(paverage);
    Serial.println("Sending data to Thingspeak");
    
    if (client.connect(server,80)) {
      String postStr = apiKey;
      postStr +="&field1=";
      postStr += String(taverage);
      postStr +="&field2=";
      postStr += String(haverage);
      postStr +="&field3=";
      postStr += String(paverage);
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
  }

  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor! Sleeping for a moment.");
    delay(20000);
    return;
  }
  
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
