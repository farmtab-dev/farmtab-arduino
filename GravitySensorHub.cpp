/*********************************************************************************************
* GravitySensorHub.cpp
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
* date    :  2017-04-12
*********************************************************************************************/

#include "GravitySensorHub.h"
#include "GravityPh.h"
#include "GravityOrp.h"
#include "GravityEc.h"
#include "GravityTDS.h"
#include "GravityTemperature.h"
#include "SensorDo.h"

//********************************************************************************************
// function name: sensors []
// Function Description: Store the array of sensors
// Parameters: 0 PH sensor
// Parameters: 1 Temperature sensor
// Parameters: 2 TDS sensor
// Parameters: 3 EC sensor
// Parameters: 4 ORP sensor
//********************************************************************************************

GravitySensorHub::GravitySensorHub()
{
	for (size_t i = 0; i < this->SensorCount; i++)
	{
		this->sensors[i] = NULL;
	}

	this->sensors[0] = new GravityPh();
	this->sensors[1] = new GravityTemperature(5);
	//this->sensors[2] = new SensorDo();
	this->sensors[2] = new GravityTDS(this->sensors[1]);
	this->sensors[3] = new GravityEc(this->sensors[1]);
	this->sensors[4] = new GravityOrp();
	this->_cmdReceivedBufferIndex = 0;
}

//********************************************************************************************
// function name: ~ GravitySensorHub ()
// Function Description: Destructor, frees all sensors
//********************************************************************************************
GravitySensorHub::~GravitySensorHub()
{
	for (size_t i = 0; i < SensorCount; i++)
	{
		if (this->sensors[i])
		{
			delete this->sensors[i];
		}
	}
}

//********************************************************************************************
// function name: setup ()
// Function Description: Initializes all sensors
//********************************************************************************************
void GravitySensorHub::setup()
{
	for (size_t i = 0; i < SensorCount; i++)
	{
		if (this->sensors[i])
		{
			this->sensors[i]->setup();
		}
	}
}

//********************************************************************************************
// function name: update ()
// Function Description: Updates all sensor values
//********************************************************************************************
void GravitySensorHub::update()
{
	for (size_t i = 0; i < SensorCount; i++)
	{
		if (this->sensors[i])
		{
			this->sensors[i]->update();
		}
	}
}

//********************************************************************************************
// function name: getValueBySensorNumber ()
// Function Description: Get the sensor data
// Parameters: 0 PH sensor
// Parameters: 1 Temperature sensor
// Parameters: 2 TDS sensor
// Parameters: 3 EC sensor
// Parameters: 4 ORP sensor
// Return Value: Returns the acquired sensor data
//********************************************************************************************
double GravitySensorHub::getValueBySensorNumber(int num)
{
	if (num >= SensorCount)
	{
		return 0;
	}
	return this->sensors[num]->getValue();
}

void GravitySensorHub::calibrate()
{
	if (cmdSerialDataAvailable() > 0)
	{
		if (strstr(this->_cmdReceivedBuffer, "ENTERPH") != NULL)
		{
			this->sensors[0]->calibration(1);
		}
		else if (strstr(this->_cmdReceivedBuffer, "EXITPH") != NULL)
		{
			this->sensors[0]->calibration(3);
		}
		else if (strstr(this->_cmdReceivedBuffer, "CALPH") != NULL)
		{
			this->sensors[0]->calibration(2);
		}
		else if (strstr(this->_cmdReceivedBuffer, "ENTERTDS") != NULL)
		{
			this->sensors[2]->calibration(1);
		}
		else if (strstr(this->_cmdReceivedBuffer, "EXITTDS") != NULL)
		{
			this->sensors[2]->calibration(3);
		}
		else if (strstr(this->_cmdReceivedBuffer, "CALTDS") != NULL)
		{
			this->sensors[2]->calibration(2);
		}
		else if (strstr(this->_cmdReceivedBuffer, "ENTEREC") != NULL)
		{
			this->sensors[3]->calibration(1);
		}
		else if (strstr(this->_cmdReceivedBuffer, "EXITEC") != NULL)
		{
			this->sensors[3]->calibration(3);
		}
		else if (strstr(this->_cmdReceivedBuffer, "CALEC") != NULL)
		{
			this->sensors[3]->calibration(2);
		}
		else
		{
			Serial.println(F(">>>Arduino Command Error<<<"));
		}
	}
}

boolean GravitySensorHub::cmdSerialDataAvailable()
{
	cmdReceivedTimeOut = millis();
	while (Serial.available() > 0)
	{
		if (millis() - cmdReceivedTimeOut > 500U)
		{
			Serial.println(F("CLEARED"));
			this->_cmdReceivedBufferIndex = 0;
			memset(this->_cmdReceivedBuffer, 0, (ReceivedBufferLength));
			Serial.println(F("CLEARED2"));
		}
		cmdReceivedTimeOut = millis();
		cmdReceivedChar = Serial.read();
		if (cmdReceivedChar == '\n' || this->_cmdReceivedBufferIndex == ReceivedBufferLength - 1)
		{
			this->_cmdReceivedBufferIndex = 0;
			strupr(this->_cmdReceivedBuffer);
			return true;
		}
		else
		{
			this->_cmdReceivedBuffer[this->_cmdReceivedBufferIndex] = cmdReceivedChar;
			this->_cmdReceivedBufferIndex++;
		}
	}
	return false;
}
