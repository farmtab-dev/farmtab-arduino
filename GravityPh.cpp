﻿/*********************************************************************
* GravityPh.cpp
*
* Copyright (C)    2017   [DFRobot](http://www.dfrobot.com),
* GitHub Link :https://github.com/DFRobot/watermonitor
* This Library is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Description:Monitoring water quality parameters ph
*
* Product Links：http://www.dfrobot.com.cn/goods-812.html
* https://www.dfrobot.com/product-2069.html
* https://github.com/DFRobot/DFRobot_PH
*https://media.digikey.com/pdf/Data%20Sheets/DFRobot%20PDFs/SEN0161_SEN0169_Web.pdf

* Sensor driver pin：A2 (phSensorPin(A2))
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-07
**********************************************************************/

#include "GravityPh.h"
#include "Arduino.h"

#include <EEPROM.h>

#define EEPROM_write(address, p)              \
    {                                         \
        int i = 0;                            \
        byte *pp = (byte *)&(p);              \
        for (; i < sizeof(p); i++)            \
            EEPROM.write(address + i, pp[i]); \
    }
#define EEPROM_read(address, p)               \
    {                                         \
        int i = 0;                            \
        byte *pp = (byte *)&(p);              \
        for (; i < sizeof(p); i++)            \
            pp[i] = EEPROM.read(address + i); \
    }

#define PHVALUEADDR 0x00 //the start address of the pH calibration parameters stored in the EEPROM

GravityPh::GravityPh() : phSensorPin(A2), offset(0.0f),
                         samplingInterval(30), pHValue(0), voltage(0), sum(0)
{
    this->_acidVoltage = 1.14;   //buffer solution 4.0 at 25C
    this->_neutralVoltage = 2.0; //buffer solution 7.0 at 25C
    readCharacteristicValues();
}

//********************************************************************************************
// function name: setup ()
// Function Description: Initializes the ph sensor
//********************************************************************************************
void GravityPh::setup()
{
    pinMode(phSensorPin, INPUT);
}

//********************************************************************************************
// function name: update ()
// Function Description: Update the sensor value
//********************************************************************************************
void GravityPh::update()
{
    static unsigned long samplingTime = millis();
    static unsigned long printTime = millis();
    static int pHArrayIndex = 0;
    if (millis() - samplingTime > samplingInterval)
    {
        samplingTime = millis();
        pHArray[pHArrayIndex++] = analogRead(this->phSensorPin);

        if (pHArrayIndex == arrayLength) // 5 * 20 = 100ms
        {
            pHArrayIndex = 0;
            for (int i = 0; i < arrayLength; i++)
                this->sum += pHArray[i];
            averageVoltage = this->sum / arrayLength;
            this->sum = 0;
            voltage = averageVoltage * 5.0 / 1024.0;
            pHValue = 3.5 * voltage + this->offset;
        }
    }
}

//********************************************************************************************
// function name: getValue ()
// Function Description: Returns the sensor data
//********************************************************************************************
double GravityPh::getValue()
{
    return this->pHValue;
}

void GravityPh::readCharacteristicValues()
{

    if (EEPROM.read(PHVALUEADDR) == 0xFF && EEPROM.read(PHVALUEADDR + 1) == 0xFF && EEPROM.read(PHVALUEADDR + 2) == 0xFF && EEPROM.read(PHVALUEADDR + 3) == 0xFF)
    {
        this->_neutralVoltage = 2.0; // new EEPROM, write typical voltage
        EEPROM_write(PHVALUEADDR, this->_neutralVoltage);
    }

    if (EEPROM.read(PHVALUEADDR + 4) == 0xFF && EEPROM.read(PHVALUEADDR + 5) == 0xFF && EEPROM.read(PHVALUEADDR + 6) == 0xFF && EEPROM.read(PHVALUEADDR + 7) == 0xFF)
    {
        this->_acidVoltage = 1.14; // new EEPROM, write typical voltage
        EEPROM_write(PHVALUEADDR + 4, this->_acidVoltage);
    }
}

void GravityPh::calibration(byte mode)
{
    phCalibration(mode); // if received Serial CMD from the serial monitor, enter into the calibration mode
}

void GravityPh::phCalibration(byte mode)
{
    char *receivedBufferPtr;
    static boolean phCalibrationFinish = 0;
    static boolean enterCalibrationFlag = 0;
    switch (mode)
    {
    case 0:
        if (enterCalibrationFlag)
        {
            Serial.println(F(">>>PH Command Error<<<"));
        }
        break;

    case 1:
        enterCalibrationFlag = 1;
        phCalibrationFinish = 0;
        Serial.println();
        Serial.println(F(">>>Enter PH Calibration Mode<<<"));
        Serial.println(F(">>>Please put the probe into the 4.0 or 7.0 standard buffer solution<<<"));
        Serial.println();
        break;

    case 2:
        if (enterCalibrationFlag)
        {
            if ((voltage > 1.7) && (voltage < 2.7))
            { // buffer solution:7.0{
                Serial.println();
                Serial.print(F(">>>PH Buffer Solution:7.0"));
                this->_neutralVoltage = voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println();
                phCalibrationFinish = 1;
            }
            else if ((voltage > 0.7) && (voltage < 1.7))
            { //buffer solution:4.0
                Serial.println();
                Serial.print(F(">>>PH Buffer Solution:4.0"));
                this->_acidVoltage = voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println();
                phCalibrationFinish = 1;
            }
            else
            {
                Serial.println();
                Serial.print(F(">>>PH Buffer Solution Error Try Again<<<"));
                Serial.println(); // not buffer solution or faulty operation
                phCalibrationFinish = 0;
            }
        }
        break;

    case 3:
        if (enterCalibrationFlag)
        {
            Serial.println();
            if (phCalibrationFinish)
            {
                if ((voltage > 1.7) && (voltage < 2.7))
                {
                    EEPROM_write(PHVALUEADDR, this->_neutralVoltage);
                }
                else if ((voltage > 0.7) && (voltage < 1.7))
                {
                    EEPROM_write(PHVALUEADDR + 4, this->_acidVoltage);
                }
                Serial.print(F(">>>PH Calibration Successful"));
            }
            else
            {
                Serial.print(F(">>>PH Calibration Failed"));
            }
            Serial.println(F(",Exit PH Calibration Mode<<<"));
            Serial.println();
            phCalibrationFinish = 0;
            enterCalibrationFlag = 0;
        }
        break;
    }
}
