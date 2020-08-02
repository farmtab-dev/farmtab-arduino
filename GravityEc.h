/*********************************************************************
* GravityEc.h
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
* Sensor driver pin：A0 (Can be modified in the .cpp file:ECsensorPin(A0);)
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-17
**********************************************************************/

#pragma once
#include "GravityTemperature.h"
#include "ISensor.h"

#define ReceivedBufferLength 10 //length of the Serial CMD buffer

// external GravityTemperature ecTemperature;

class GravityEc:public ISensor
{
public:
	// Conductivity sensor pin
	int ecSensorPin;

	// Conductivity values
	double ECcurrent;
	double ECvalueRaw;
	double TempCoefficient; 

	double CoefficientVolatge ;
	float compensationFactor;

public:
	GravityEc(ISensor*);
	~GravityEc();

	// initialization
	void  setup ();

	// update the sensor data
	void  update ();

	// Get the sensor data
	double getValue();


	// Added from DFRobot_EC
	// void calibration(char *cmd); //calibration by Serial CMD
	// void calibration();			   //calibration by Serial CMD
	void calibration(byte mode);			   //calibration by Serial CMD
	void readCharacteristicValues();

private:
	// point to the temperature sensor pointer
	ISensor* ecTemperature = NULL;


	static const int numReadings = 5;
	unsigned int readings[numReadings] = { 0 };      // the readings from the analog input
	int index;
	double sum;
	unsigned long AnalogValueTotal;      // the running total
	unsigned int AnalogAverage;
	unsigned int averageVoltage;
	unsigned long AnalogSampleTime;
	unsigned long printTime;
	unsigned  long tempSampleTime;
	unsigned long AnalogSampleInterval;
	unsigned long printInterval ;

	// Added from DFRobot_EC
	float _kvalue;
	float _kvalueLow;
	float _kvalueHigh;
  char  _cmdReceivedBuffer[ReceivedBufferLength];  //store the Serial CMD
  byte  _cmdReceivedBufferIndex;
	
	// Calculate the average
  void calculateAnalogAverage();

	// Calculate the conductivity
	void calculateEc();

	//Added from DFRobot_EC
	//boolean cmdSerialDataAvailable();
	void    ecCalibration(byte mode); // calibration process, wirte key parameters to EEPROM
	//byte    cmdParse(const char *cmd);
	//byte    cmdParse();
};
