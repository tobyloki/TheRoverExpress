#include <Wire.h>
#include "DHT.h"  //for temp/humidity 
#include "Seeed_BMP280.h"
#include <Arduino.h>
#include <U8x8lib.h>  //for OLED

#define DHTPIN 3     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11 
DHT dht(DHTPIN, DHTTYPE);  // temperature and humidity sensor
BMP280 bmp280; //air pressure sensor
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
// Light Switch
int sensorpin = A6; // Analog input pin that the sensor is attached to
int ledPin = 4;    // LED port
int sensorValue = 0;        // value read from the port
int outputValue = 0;        // value output to the PWM (analog out)
int readyForTransmitPin = 13;

float mTemp = 20;
float mHumi = 52.5;
float mPressure = 100356.625;

int sensorPinT = A0;    // The potentiometer is connected to analog pin 0                  
int sensorValueT;    // We declare another integer variable to store the value of the potentiometer
String inData;

void setup(void) {
  Serial.begin(9600); 
  // set up OLED
  u8x8.begin();
  u8x8.setPowerSave(0);  
  u8x8.setFlipMode(1);
  // set up temperature and humidity sensor
  dht.begin();
  // set up air pressure sensor
  if (!bmp280.init()) {
      Serial.println("Device not connected or broken!");
  }
  // set up LED light
  pinMode(ledPin,OUTPUT);
  // set up light sensor
  pinMode(sensorpin, INPUT);
}
 
void loop(void) {
  float temp, humi, pressure;
  temp = dht.readTemperature();
  humi = dht.readHumidity();
  pressure = bmp280.getPressure();
  sensorValue = analogRead(sensorpin);
 
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 33);

  //Get temperature and humidity data
  u8x8.print("Temp:");
  u8x8.print(temp);
  u8x8.println("C");
  u8x8.setCursor(0,50);
  u8x8.print("Humidity:");
  u8x8.print(humi);
  u8x8.println("%");
  //get and print atmospheric pressure data
  u8x8.print("Pre:");
  u8x8.print(pressure);
  u8x8.println("Pa");
  //Calculate Habitability Index Score
  float score = calculateHabitabilityIndex(temp, humi, pressure);
  u8x8.print("Score:");
  u8x8.print(score, 2);
  u8x8.println("%");
  //Print light sensor results
  u8x8.print("Light:");
  u8x8.println(sensorValue);
  if (sensorValue > 300) {  //using light value to simulate if battery is low
    digitalWrite(ledPin, HIGH);  //turn LED on
    u8x8.println("Rover Charging");  //print charge needed
  }
  else {
    digitalWrite(ledPin, LOW);  //turn LED off
    u8x8.println("             ");  //clear OLED output
  }
  // Transmit Data
  Serial.print("temp:");
  Serial.println(temp);
  Serial.print("humidity:");
  Serial.println(humi);
  Serial.print("air:");
  Serial.println(pressure);
  Serial.print("score:");
  Serial.println(score);

  sensorValueT = analogRead(sensorPinT);
  while (Serial.available() > 0)
    {
        char recieved = Serial.read();
        inData += recieved; 

        // Process message when new line character is recieved
        if (recieved == '\n')
        {
            int colonPosition = inData.indexOf(':');
            if(colonPosition > -1){
              float value = (inData.substring(colonPosition+1)).toFloat();
              if(inData.startsWith("airThreshold")){
                mPressure = value;
              } else if(inData.startsWith("tempThreshold")){
                mTemp = value;
              } else if(inData.startsWith("humidityThreshold")){
                mHumi = value;
              } 
            }
            inData = ""; // Clear recieved buffer
        }
    }
  
  u8x8.refreshDisplay();
  delay(2000);
}


float calculateHabitabilityIndex(float temp, float humi, float pres){
    float tempLow[4] = {mTemp*0.5, mTemp*0.4, mTemp*0.3, mTemp*0.2}; 
    float tempHigh[4] = {mTemp*1.5, mTemp*1.6, mTemp*1.7, mTemp*1.9}; 
    int tempScore = calculateScore(temp, tempLow, tempHigh);
    float humiLow[4] = {mHumi*0.5, mHumi*0.4, mHumi*0.3, mHumi*0.2}; 
    float humiHigh[4] = {mHumi*1.5, mHumi*1.6, mHumi*1.7, mHumi*1.9}; 
    int humiScore = calculateScore(humi, humiLow, humiHigh);
    float presLow[4] = {mPressure*0.5, mPressure*0.4, mPressure*0.3, mPressure*0.2}; 
    float presHigh[4] = {mPressure*1.5, mPressure*1.6, mPressure*1.7, mPressure*1.9}; 
    int presScore = calculateScore(pres, presLow, presHigh);
    bool badScores[3] = {false, false, false};
    calculateBadScores(badScores, tempScore, humiScore, presScore);
    int scores[3] = {tempScore, humiScore, presScore};
    float weights[3] = {0.333333, 0.333333, 0.333333};
    for (int i = 0; i < 3; i++) {
        if (badScores[i]) {
            for (int x = 0; x < 3; x++) {
                if (x == i)
                    weights[i] += 0.1;
                else
                    weights[x] -= 0.05;
            }
        }
    }
    float aggregateScore = 0;
    for (int i = 0; i < 3; i++) {
        aggregateScore += weights[i] * (scores[i] - 1);
    }
    aggregateScore = ((aggregateScore / 4) * 100);
    return aggregateScore;
}

int calculateScore(float value, float low[], float high[]){
  int score = 5;
  for (int x = 0; x < 4; x++) {  //check if score should be 5-x
    if (value <= high[x] && value >= low[x])
      break;
    score--;
  }
  return score;
}
void calculateBadScores(bool badScores[], int tempScore,int humiScore, int presScore){
  int scores[3] = {tempScore, humiScore, presScore};
  for (int i = 0; i < 3; i++) {
    if (scores[i] <= 3) {
      float average = 0;
      for (int x = 0; x < 3; x++)
        if (x != i)
          average += scores[x];
        average = (average / 2) - 0.5;
      if (scores[i] < average) {
        badScores[i] = true;
      }
    }
  }
}
