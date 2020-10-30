/*********************************************************************
* GravityEc.cpp
*
* Copyright (C)    2017   [DFRobot](http://www.dfrobot.com),
* GitHub Link :https://github.com/DFRobot/watermonitor
* This Library is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Description:Monitoring water quality parameters Conductivity
*
* Product Links：http://www.dfrobot.com.cn/goods-882.html
*
* Sensor driver pin：A0 (ecSensorPin(A0))
* https://wiki.dfrobot.com/Gravity__Analog_Electrical_Conductivity_Sensor___Meter_V2__K%3D1__SKU_DFR0300
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-17
**********************************************************************/

#include "GravityEc.h"
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

#define compensationFactorAddress 8 //the address of the factor stored in the EEPROM

GravityEc::GravityEc(ISensor *temp) : ecSensorPin(A0), ECcurrent(0), index(0), AnalogAverage(0),
                                      AnalogValueTotal(0), averageVoltage(0), AnalogSampleTime(0), printTime(0), sum(0),
                                      tempSampleTime(0), AnalogSampleInterval(25), printInterval(700)
{
    this->ecTemperature = temp;
    this->_cmdReceivedBufferIndex = 0;
    readCharacteristicValues(); //read the compensationFactor
}

GravityEc::~GravityEc() {}

//********************************************************************************************
// function name: setup ()
// Function Description: Initializes the sensor
//********************************************************************************************
void GravityEc::setup()
{
    pinMode(ecSensorPin, INPUT);
    for (byte thisReading = 0; thisReading < numReadings; thisReading++)
        readings[thisReading] = 0;
}

//********************************************************************************************
// function name: update ()
// Function Description: Update the sensor value
//********************************************************************************************
void GravityEc::update()
{
    calculateAnalogAverage();
    calculateEc();
}

//********************************************************************************************
// function name: getValue ()
// Function Description: Returns the sensor data
//********************************************************************************************
double GravityEc::getValue()
{
    return ECcurrent;
}

//********************************************************************************************
// function name: calculateAnalogAverage ()
// Function Description: Calculates the average voltage
//********************************************************************************************
void GravityEc::calculateAnalogAverage()
{
    if (millis() - AnalogSampleTime >= AnalogSampleInterval)
    {
        AnalogSampleTime = millis();
        readings[index++] = analogRead(ecSensorPin);
        if (index == numReadings)
        {
            index = 0;
            for (int i = 0; i < numReadings; i++)
                this->sum += readings[i];
            AnalogAverage = this->sum / numReadings;
            this->sum = 0;
        }
    }
}

//********************************************************************************************
// function name: calculateAnalogAverage ()
// Function Description: Calculate the conductivity
//********************************************************************************************
void GravityEc::calculateEc()
{
    if (millis() - printTime >= printInterval)
    {
        printTime = millis();
        averageVoltage = AnalogAverage * 5000.0 / 1024.0;
        TempCoefficient = 1.0 + 0.0185 * (this->ecTemperature->getValue() - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.0185*(fTP-25.0));

        CoefficientVolatge = (double)averageVoltage / TempCoefficient;

        if (CoefficientVolatge < 150)
        {
            ECcurrent = 0;
            return;
        }
        else if (CoefficientVolatge > 3300)
        {
            ECcurrent = 20;
            return;
        }
        else
        {
            if (CoefficientVolatge <= 448)
                ECcurrent = 6.84 * CoefficientVolatge - 64.32; //1ms/cm<EC<=3ms/cm
            else if (CoefficientVolatge <= 1457)
                ECcurrent = 6.98 * CoefficientVolatge - 127; //3ms/cm<EC<=10ms/cm
            else
                ECcurrent = 5.3 * CoefficientVolatge + 2278; //10ms/cm<EC<20ms/cm
            //ECcurrent /= 1000;
            ECvalueRaw = ECcurrent / 1000.0;                     //convert us/cm to ms/cm
            ECcurrent = ECcurrent / compensationFactor / 1000.0; //after compensation,convert us/cm to ms/cm
        }
    }
}

/*************************************
 * READ compensationFactor
 *************************************/
void GravityEc::readCharacteristicValues()
{
    EEPROM_read(compensationFactorAddress, compensationFactor);
    if (EEPROM.read(compensationFactorAddress) == 0xFF && EEPROM.read(compensationFactorAddress + 1) == 0xFF && EEPROM.read(compensationFactorAddress + 2) == 0xFF && EEPROM.read(compensationFactorAddress + 3) == 0xFF)
    {
        compensationFactor = 1.0; // If the EEPROM is new, the compensationFactorAddress is 1.0(default).
        EEPROM_write(compensationFactorAddress, compensationFactor);
    }
}

void GravityEc::calibration(byte mode)
{
    ecCalibration(mode); // if received Serial CMD from the serial monitor, enter into the calibration mode
}

/******************************************
 * EC CALIBRATION
 ******************************************/
void GravityEc::ecCalibration(byte mode)
{
    char *receivedBufferPtr;
    static boolean ecCalibrationFinish = 0;
    static boolean enterCalibrationFlag = 0;
    // static float compECsolution;
    float factorTemp;
    switch (mode)
    {
    case 0:
        if (enterCalibrationFlag)
        {
            Serial.println(F(">>>EC Command Error<<<"));
        }
        break;
    case 1:
        enterCalibrationFlag = 1;
        ecCalibrationFinish = 0;
        Serial.println();
        Serial.println(F(">>>Enter EC Calibration Mode<<<"));
        Serial.println(F(">>>Please put the probe into the 1.413ms/cm buffer solution<<<"));
        Serial.println();
        break;
    case 2:
        if (enterCalibrationFlag)
        {
            factorTemp = ECvalueRaw / 1.413;
            if ((factorTemp > 0) && (factorTemp < 1.15))
            {
                Serial.println();
                Serial.println(F(">>>Confirm Successful<<<"));
                Serial.println();
                compensationFactor = factorTemp;
                ecCalibrationFinish = 1;
            }
            else
            {
                Serial.println();
                Serial.println(F(">>>Confirm Failed,Try Again<<<"));
                Serial.println();
                ecCalibrationFinish = 0;
            }
        }
        break;
    case 3:
        if (enterCalibrationFlag)
        {
            Serial.println();
            if (ecCalibrationFinish)
            {
                EEPROM_write(compensationFactorAddress, compensationFactor);
                Serial.print(F(">>>Calibration Successful"));
            }
            else
            {
                Serial.print(F(">>>Calibration Failed"));
            }
            Serial.println(F(",Exit EC Calibration Mode<<<"));
            Serial.println();
            ecCalibrationFinish = 0;
            enterCalibrationFlag = 0;
        }
        break;
    }
}
