#include <EloquentTinyML.h>
#include <eloquent_tinyml/tensorflow.h>

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

unsigned long myChannelNumber = 1920297;
const char * myWriteAPIKey = "JG4XUD3YPQQBYD93";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 900000;
int entries = 0;

// sine_model.h contains the array you exported from Python with xxd or tinymlgen
#include "dense_model-10m.h"

#define N_INPUTS 5
#define N_OUTPUTS 1
// in future projects you may need to tweak this value: it's a trial and error process
#define TENSOR_ARENA_SIZE 12*1024

Eloquent::TinyML::TensorFlow::TensorFlow<N_INPUTS, N_OUTPUTS, TENSOR_ARENA_SIZE> tf;

float threshold = 0.75;
float temp_readings[5];
float predicted_temp;

int arrSize = *(&temp_readings + 1) - temp_readings;

// Define the timer for the data collection.
unsigned long sensor_timer = 0;
unsigned long timer = 0;

// Define the data holders: 
float _temperature, _pressure, _humidity;

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);   
    ThingSpeak.begin(client);  // Initialize ThingSpeak

    // Initialize the SSD1306 screen:
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.display();
    delay(1000);
  
    if (!bme.begin()) {
      Serial.println("Could not find a valid BME680 sensor, check wiring!");
      while (1);
    }
  
    sensor_timer = millis();
  
    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms
  
    Serial.println("\nSensors connected successfully!\n");
  
    delay(4000);
    tf.begin(dense_model_10m_tflite);

    // check if model loaded fine
    if (!tf.isOk()) {
      Serial.print("ERROR: ");
      Serial.println(tf.getErrorMessage());
      
      delay(1000);
      display.clearDisplay();   
      display.setTextSize(1); 
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println(tf.getErrorMessage());
      display.display();

      while (true) delay(1000);
    }

    Serial.println("Setup Complete!");

    delay(1000);
    display.clearDisplay();   
    display.setTextSize(1); 
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Setup Complete!");
    display.display();

}

void loop() {
    // put your main code here, to run repeatedly:
    Serial.print("millis(): "); Serial.println(millis());
    Serial.print("timer: "); Serial.println(sensor_timer);
    Serial.print("temp: "); Serial.println(bme.temperature);
  
    if (! bme.performReading()) {
      Serial.println("Failed to perform reading :(");
      return;
    }
  
    if ((millis()-sensor_timer) < 60*1000) {
      Serial.println("Loading temp data...");
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Loading temp data...");
      display.display();
      load_temp_readings();
    }
    else {
      update_temp_readings();
    }
  
    display.clearDisplay();
    display.setCursor(0,0);
  
    run_inference_to_make_predictions();
//    delay(timerDelay);
//    upload();
//    collect_BME680_data();
    delay(1000);  

}

void load_temp_readings() {
  int i = arrSize-1;
  Serial.print("size: "); Serial.println(i);
  while (i >= 0) {
    temp_readings[i] = bme.temperature/100.0;
    Serial.print("Loading..."); Serial.println(bme.temperature);
    i--;
    delay(20000);
  }
  Serial.println("Loading Complete!");
}

void update_temp_readings() {
  
  Serial.println("Updating Sensor data...");
  int i = arrSize-1;
  Serial.print("size: "); Serial.println(i);
  
  while (i >= 0) {
    if (i == 0) {
      temp_readings[i] = bme.temperature/100.0;
      i--;
    }
    else {
      temp_readings[i] = temp_readings[i-1];
      i--;
    }
  }
}

void run_inference_to_make_predictions(){    
    // Scale (normalize) values (local weather data) depending on the model and copy them to the input buffer (tensor):
//    model_input->data.f[0] = _temperature / 100;

    unsigned long t1 = micros();
    float predicted_temp = tf.predict(temp_readings)*100;
    unsigned long t2 = micros();
    
    // Read predicted y values (air quality classes) from the output buffer (tensor): 
//    predicted_temp = model_output->data.f[0];
    Serial.print("\nExecution Time: ");
    Serial.print(t2-t1);
    Serial.println(" microseconds\n"); 
    Serial.print("T1, T2: "); Serial.print(t1); Serial.print(", "); Serial.print(t2); Serial.println("\n");
    Serial.print("\nPredicted output: "); Serial.print(predicted_temp); Serial.println("\n");
    // Exit and clear.
    delay(3000);
    Serial.print("Temperature: "); Serial.print(bme.temperature); Serial.println(" *C");
    Serial.print("Pressure: "); Serial.print(bme.pressure/100.0); Serial.println(" hPa");
    Serial.print("Humidity: "); Serial.print(bme.humidity); Serial.println(" %");
    Serial.print("Predicted temp: "); Serial.print(predicted_temp); Serial.println(" *C");
  
    display.print("Temperature: "); display.print(bme.temperature); display.println(" *C");
    display.print("Pressure: "); display.print(bme.pressure/100.0); display.println(" hPa");
    display.print("Humidity: "); display.print(bme.humidity); display.println(" %");
    display.print("Predicted temp: "); display.print(predicted_temp); display.println(" *C");
    display.display();

    
    delay(timerDelay);

    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

    float pressure = bme.pressure / 100.0;

    ThingSpeak.setField(1, bme.temperature);
    //ThingSpeak.setField(1, temperatureF);
    ThingSpeak.setField(2, bme.humidity);
    ThingSpeak.setField(3, pressure);
    ThingSpeak.setField(4, predicted_temp);

    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful.");
      entries += 1;
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Problem updating channel. HTTP error code " + String(x));
      display.display();
      delay(10000);
    }
    
}

void upload() {
  
}

void collect_BME680_data(){
  _temperature = bme.temperature;
  _pressure = bme.pressure;
  // Calculate altitude assuming 'standard' barometric pressure of 1013.25 millibars (101325 Pascal).
  _humidity = bme.humidity;
//  _sea_level_pressure = bmp.readSealevelPressure();
  
  // Print the data generated by the BMP180 Barometric Pressure/Temperature/Altitude Sensor.
  Serial.print("Temperature: "); Serial.print(bme.temperature); Serial.println(" *C");
  Serial.print("Pressure: "); Serial.print(bme.pressure/100.0); Serial.println(" hPa");
  Serial.print("Humidity: "); Serial.print(bme.humidity); Serial.println(" %");
  Serial.print("Predicted temp: "); Serial.print(predicted_temp); Serial.println(" *C");

  display.print("Temperature: "); display.print(bme.temperature); display.println(" *C");
  display.print("Pressure: "); display.print(bme.pressure/100.0); display.println(" hPa");
  display.print("Humidity: "); display.print(bme.humidity); display.println(" %");
  display.print("Predicted temp: "); display.print(predicted_temp); display.println(" *C");
  display.display();
}
