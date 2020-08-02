/*********************************************************************
* GravitySensorHub.h
*
* Copyright (C)    2017   [DFRobot](http://www.dfrobot.com),
* GitHub Link :https://github.com/DFRobot/watermonitor
* This Library is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Description:
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-07
**********************************************************************/

#pragma once
#include "ISensor.h"
#define ReceivedBufferLength 10 //length of the Serial CMD buffer
/*
sensors :
0,ph
1,ec
2.orp
*/
class GravitySensorHub
{
private:
	static const int SensorCount = 10;
	char cmdReceivedChar;
	unsigned long cmdReceivedTimeOut;
	//long cmdReceivedTimeOut;
	char _cmdReceivedBuffer[ReceivedBufferLength]; //store the Serial CMD
	byte _cmdReceivedBufferIndex;
	boolean cmdSerialDataAvailable();

public:
	void calibration(byte mode);
	//********************************************************************************************
	// function name: sensors []
	// Function Description: Store the array of sensors
	// Parameters: 0 PH sensor
	// Parameters: 1 Temperature sensor
	// Parameters: 2 TDS sensor
	// Parameters: 3 EC sensor
	// Parameters: 4 ORP sensor
	//********************************************************************************************
	ISensor *sensors[SensorCount] = {0};
public:
	GravitySensorHub();
	~GravitySensorHub();

	// initialize all sensors
	void  setup ();

	// update all sensor values
	void  update ();

	// Get the sensor data
	double getValueBySensorNumber(int num);
	void calibrate();
};
