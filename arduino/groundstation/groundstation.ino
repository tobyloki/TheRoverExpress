/*
  Firmata is a generic protocol for communicating with microcontrollers
  from software on a host computer. It is intended to work with
  any host computer software package.

  To download a host software package, please click on the following link
  to open the list of Firmata client libraries in your default browser.

  https://github.com/firmata/arduino#firmata-client-libraries

  Copyright (C) 2006-2008 Hans-Christoph Steiner.  All rights reserved.
  Copyright (C) 2010-2011 Paul Stoffregen.  All rights reserved.
  Copyright (C) 2009 Shigeru Kobayashi.  All rights reserved.
  Copyright (C) 2009-2016 Jeff Hoefs.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.

  Last updated August 17th, 2017
*/

#include <Servo.h>
#include <Wire.h>
#include <Firmata.h>

#define I2C_WRITE                   B00000000
#define I2C_READ                    B00001000
#define I2C_READ_CONTINUOUSLY       B00010000
#define I2C_STOP_READING            B00011000
#define I2C_READ_WRITE_MODE_MASK    B00011000
#define I2C_10BIT_ADDRESS_MODE_MASK B00100000
#define I2C_END_TX_MASK             B01000000
#define I2C_STOP_TX                 1
#define I2C_RESTART_TX              0
#define I2C_MAX_QUERIES             8
#define I2C_REGISTER_NOT_SPECIFIED  -1

#include <Arduino.h>
#define btn1 6
#define btn2 7
#define btn3 8
#define led 4
#define ROTARY_ANGLE_SENSOR A0
#define ADC_REF 5
#define FULL_ANGLE 300
#define GROVE_VCC 5
#include <U8x8lib.h> 
//u8g2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(u8g2_R2, SCL, SDA, U8X8_PIN_NONE);  // high speed I2C
int btn1State;
int btn2State;
int btn3State;
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

void setup(void) {
  Serial.begin(9600);
  pinMode(btn1, INPUT);
  pinMode(btn2, INPUT);
  pinMode(btn3, INPUT);
  pinMode(led, OUTPUT);
  u8x8.begin();
  u8x8.setPowerSave(0);  
  u8x8.setFlipMode(1);
}

bool LongPress(int timeunit) {
  int longpress = 0;
  for (int i = 0; i < timeunit; i++) {
    btn1State = digitalRead(btn1);
    if (digitalRead(btn1) == HIGH) {
      longpress++;
    }
    delay(50);
  }
  return (longpress == timeunit);
}

int ChangeThreshold(int select) {
  int val;
  while (1) {
    digitalWrite(led, HIGH);
    delay(50);
    int sensor_value = analogRead(ROTARY_ANGLE_SENSOR);
    float voltage = (float)sensor_value * ADC_REF / 1023;
    float degrees = (voltage * FULL_ANGLE) / GROVE_VCC;
    if (select==1) {
      val = map(degrees, 0, FULL_ANGLE, 0, 100);
      val = 150 - val;
    }
    else if (!select) {
      val = map(degrees, 0, FULL_ANGLE, 0, 100);
      val = 150 - val;
    }
    else {
      val = map(degrees, 0, FULL_ANGLE, 0, 50);
      val = 50 - val;
    }
    digitalWrite(led, LOW);
    delay(20);
    if (digitalRead(btn1) == HIGH) {
      return val;
    }
  }
}

int Edit(int rover) {
  if(!rover){
      rover= 2;
    }
  int first = 1;
  static int select = 3, deftemp = 42, defair = 49, defhum = 56;
  u8x8.println("            ");
  u8x8.println("            ");
  u8x8.println("            ");
  u8x8.println("            ");
  u8x8.println("            ");
  u8x8.println("            ");
  u8x8.println("            ");
  u8x8.println("            ");
  u8x8.println("            ");
  u8x8.println("            ");
  u8x8.println("            ");
  u8x8.println("            ");
  while (1) {
    digitalWrite(led, HIGH);
    float voltage;
    int sensor_value = analogRead(ROTARY_ANGLE_SENSOR);
    voltage = (float)sensor_value * ADC_REF / 1023;
    float degrees = (voltage * FULL_ANGLE) / GROVE_VCC;
    int newselect = map(degrees, 0, FULL_ANGLE, 0, 29);
    if ((int)newselect / 10 != select) {
      u8x8.refreshDisplay();
      u8x8.setFont(u8x8_font_chroma48medium8_r);
      u8x8.setCursor(0,50);
      u8x8.print("thresholdH:");
      u8x8.println(defhum);
      u8x8.print("thresholdA:");
      u8x8.print(defair);
      u8x8.println("Pa");
      u8x8.print("thresholdT:");
      u8x8.print(deftemp);
      u8x8.println("C");
      select = (int)newselect / 10;
    }
    if(select==2){digitalWrite(led, LOW); delay(50);}
    else if(select){digitalWrite(led, LOW); delay(150);}
    if(!select){digitalWrite(led, LOW); delay(250);}
    if(first){
      first = 0;
      delay(300);
    }
    if (digitalRead(btn1) == HIGH) {
      if (LongPress(8)) {
        return 1;
      }
      if (select == 0) {
        defhum = ChangeThreshold(select);
        Serial.print("rover:rover");
        Serial.print(rover);
        Serial.print(",humidityThreshold:");
        Serial.println(defhum);
      }
      else if (select == 1) {
        defair = ChangeThreshold(select);
        Serial.print("rover:rover");
        Serial.print(rover);
        Serial.print(",airThreshold:");
        Serial.println(defair);
      }
      else {
        deftemp = ChangeThreshold(select);
        Serial.print("rover:rover");
        Serial.print(rover);
        Serial.print(",tempThreshold:");
        Serial.println(deftemp);
      }
      delay(100);
    }
  }
}

String inData;
float air=0, temp=0, humi=0, habi=0;

/*==============================================================================
 * LOOP()
 *============================================================================*/
void loop()
{
  static int mode = 1, load = 1;
  btn2State = digitalRead(btn2);
  btn3State = digitalRead(btn3);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 33);
  u8x8.print("Rover");
  if(mode){
    u8x8.println(" one");
    }
  else{
    u8x8.println(" two");
  }
  u8x8.setCursor(0,50);
  u8x8.print("Temp:");
  u8x8.print(temp);
  u8x8.println("C");
  u8x8.print("Pre: ");
  u8x8.print(air);
  u8x8.println("Pa");
  u8x8.print("Humidity:");
  u8x8.println(humi);
  u8x8.print("Score: ");
  u8x8.print(habi, 2);
  u8x8.println("%");
  while (Serial.available() > 0)
    {
        char recieved = Serial.read();
        inData += recieved; 
        if (recieved == '\n')
        {
            Serial.println(inData);
            int colonPosition = inData.indexOf(':');
            if(colonPosition > -1){
              float value = (inData.substring(colonPosition+1)).toFloat();
              if(inData.startsWith("air")){
                air = value;
              } else if(inData.startsWith("temp")){
                temp = value;
              } else if(inData.startsWith("humidity")){
                humi = value;
              } else if(inData.startsWith("score")){
                habi = value;
              } 
            }
            inData = ""; // Clear recieved buffer
        }
    }
  if (btn2State == LOW || (load && mode)) {
    digitalWrite(led, HIGH);
    mode = 1;
    load = 0;
  }

  else if (btn3State == LOW || (load && !mode)) {
    digitalWrite(led, LOW);
    mode = 0;
    load = 0;
  }
  if (digitalRead(btn1) == HIGH && LongPress(8)) {
    load = Edit(mode);
  }
}
