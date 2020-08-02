/*********************************************************************
   WaterMonitor.ino

   Copyright (C)    2017   [DFRobot](http://www.dfrobot.com)
   GitHub Link :https://github.com/DFRobot/watermonitor
   This Library is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Description:
   This sample code is mainly used to monitor water quality
   including ph, temperature, dissolved oxygen, ec and orp,etc.

   Software Environment: Arduino IDE 1.8.2
   Software download link: https://www.arduino.cc/en/Main/Software

   Install the library file：
   Copy the files from the github repository folder libraries to the libraries
   in the Arduino IDE 1.8.2 installation directory

   Hardware platform   : Arduino M0 Or Arduino Mega2560
   Sensor pin:
   EC  : A1
   PH  : A2
   ORP : A3
   RTC : I2C
   DO  : Serial port Rx(0),Tx(1)
   GravityDO：A4
   temperature:D5

   SD card attached to SPI bus as follows:
   Mega:  MOSI - pin 51, MISO - pin 50, CLK - pin 52, CS - pin 53
   and pin #53 (SS) must be an output
   M0:   Onboard SPI pin,CS - pin 4 (CS pin can be changed)

   author  :  Jason(jason.ling@dfrobot.com)
   version :  V1.0
   date    :  2017-04-06
 **********************************************************************/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "GravitySensorHub.h"
#include "GravityRtc.h"
#include "OneWire.h"
#include "SdService.h"
#include "Debug.h"
#include <SoftwareSerial.h>

// clock module
GravityRtc rtc;

// sensor monitor
GravitySensorHub sensorHub;
SdService sdService = SdService(sensorHub.sensors);
int WATER_LEVEL_PIN1 = 8; //Digital pin 8
int WATER_LEVEL_PIN2 = 9; //Digital pin 9
int WATER_LEVEL_PIN3 = 10; //Digital pin 9
void setup() {
  Serial.begin(9600);
  pinMode(WATER_LEVEL_PIN1, INPUT);
  pinMode(WATER_LEVEL_PIN2, INPUT);
  pinMode(WATER_LEVEL_PIN3, INPUT);
  rtc.setup();
  sensorHub.setup();
  sdService.setup();

}


//********************************************************************************************
// function name: sensorHub.getValueBySensorNumber (0)
// Function Description: Get the sensor's values, and the different parameters represent the acquisition of different sensor data
// Parameters: 0 ph value
// Parameters: 1 temperature value
// Parameters: 2 Dissolved Oxygen
// Parameters: 3 Conductivity
// Parameters: 4 Redox potential
// return value: returns a double type of data
//********************************************************************************************

unsigned long updateTime = 0;

void loop() {
  rtc.update();
  sensorHub.update();
  sdService.update();

  // ************************* Serial debugging ******************
  if (millis() - updateTime > 3000)
  {
    updateTime = millis();
    Serial.print(F("PH@"));
    Serial.print(sensorHub.getValueBySensorNumber(0));
    Serial.print(F("#TEMP@"));
    Serial.print(sensorHub.getValueBySensorNumber(1));
    Serial.print(F("#TDS@"));
    Serial.print(sensorHub.getValueBySensorNumber(2));
    Serial.print(F("#EC@"));
    Serial.print(sensorHub.getValueBySensorNumber(3));
    Serial.print(F("#ORP@"));
    Serial.print(sensorHub.getValueBySensorNumber(4));
    Serial.print(F("#WLVL1@"));
    Serial.print(digitalRead(WATER_LEVEL_PIN1));
    Serial.print(F("#WLVL2@"));
    Serial.print(digitalRead(WATER_LEVEL_PIN2));
    Serial.print(F("#WLVL3@"));
    Serial.println(digitalRead(WATER_LEVEL_PIN3));
  }
  sensorHub.calibrate();

}



//* ***************************** Print the relevant debugging information ************** ************ * /
// Note: Arduino M0 need to replace Serial with SerialUSB when printing debugging information

// ************************* Serial debugging ******************
//Serial.print("ph= ");
//Serial.print(sensorHub.getValueBySensorNumber(0));
//Serial.print("  Temp= ");
//Serial.print(sensorHub.getValueBySensorNumber(1));
//Serial.print("  Orp= ");
//Serial.println(sensorHub.getValueBySensorNumber(4));
//Serial.print("  EC= ");
//Serial.println(sensorHub.getValueBySensorNumber(3));


// ************************************************************ time ********************** **********
//Serial.print("   Year = ");//year
//Serial.print(rtc.year);
//Serial.print("   Month = ");//month
//Serial.print(rtc.month);
//Serial.print("   Day = ");//day
//Serial.print(rtc.day);
//Serial.print("   Week = ");//week
//Serial.print(rtc.week);
//Serial.print("   Hour = ");//hour
//Serial.print(rtc.hour);
//Serial.print("   Minute = ");//minute
//Serial.print(rtc.minute);
//Serial.print("   Second = ");//second
//Serial.println(rtc.second);
