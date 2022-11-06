/*
  Adapted from WriteSingleField Example from ThingSpeak Library (Mathworks)
  
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-thingspeak-publish-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include "ThingSpeak.h"

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_BME680 bme; // I2C
//Adafruit_BME680 bme(BME_CS); // hardware SPI
//Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO,  BME_SCK);

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

#define SEALEVELPRESSURE_HPA (1013.25)

const char* ssid = "V.Haus";   // your network SSID (name) 
const char* password = "MetierTshivhase1";   // your network password

WiFiClient  client;

unsigned long myChannelNumber = 1915407;
const char * myWriteAPIKey = "130AJ52DC5N5P51Q";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 600000;
int entries = 0;

// Variable to hold temperature readings
float temperatureC;

void setup() {
  Serial.begin(115200);  //Initialize serial
  
  // Initialize the SSD1306 screen:
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

//  sensor_timer = millis();

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  Serial.println("\nSensors connected successfully!\n");
  
  WiFi.mode(WIFI_STA);   
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  display.clearDisplay();   
  display.setTextSize(1); 
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Setup Complete!");
  display.display();
}

void loop() {

  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  if ((millis() - lastTime) > timerDelay) {
    
    // Connect or reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

    // Get a new temperature reading
    temperatureC = bme.temperature;
//    float pressure = bme.pressure/100.0;
    Serial.print("Temperature (ºC): ");
    Serial.println(temperatureC);
    float humidity = bme.readHumidity();
    Serial.print("Humidity (%): ");
    Serial.println(humidity);
    float pressure = bme.pressure / 100.0;
    Serial.print("Pressure (hPa): ");
    Serial.println(pressure);
    float gas = bme.gas_resistance;
    Serial.print("Pressure (hPa): ");
    Serial.println(gas);
    
    //uncomment if you want to get temperature in Fahrenheit
    /*temperatureF = 1.8 * bme.readTemperature() + 32;
    Serial.print("Temperature (ºC): ");
    Serial.println(temperatureF);*/
    
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("Temperature: "); display.print(bme.temperature); display.println(" *C");
    display.print("Pressure: "); display.print(bme.pressure/100.0); display.println(" hPa");
    display.print("Humidity: "); display.print(bme.humidity); display.println(" %");
//    display.print("Predicted temp: "); display.print(predicted_temp); display.println(" *C");
    display.display();    

    ThingSpeak.setField(1, bme.temperature);
    //ThingSpeak.setField(1, temperatureF);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, pressure);
    ThingSpeak.setField(4,gas); 
       
    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    
    //uncomment if you want to get temperature in Fahrenheit
    //int x = ThingSpeak.writeField(myChannelNumber, 1, temperatureF, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful.");
      entries += 1;
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  }
  else{
    display.clearDisplay();   
  display.setTextSize(1); 
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Entries: ");
  display.println(entries);
  display.display();
  }
}
