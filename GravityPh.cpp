/*********************************************************************
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
*
* Sensor driver pin：A2 (phSensorPin(A2))
*
* author  :  Jason(jason.ling@dfrobot.com)
* version :  V1.0
* date    :  2017-04-07
**********************************************************************/

#include "GravityPh.h"
#include "Arduino.h"

#include <EEPROM.h>

#define EEPROM_write(address, p) {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) EEPROM.write(address+i, pp[i]);}
#define EEPROM_read(address, p)  {int i = 0; byte *pp = (byte*)&(p);for(; i < sizeof(p); i++) pp[i]=EEPROM.read(address+i);}

#define PHVALUEADDR 0x00    //the start address of the pH calibration parameters stored in the EEPROM

GravityPh::GravityPh():phSensorPin(A2), offset(0.0f), 
                       samplingInterval(30),pHValue(0),voltage(0), sum(0)
{
    this->_acidVoltage    = 1.14;    //buffer solution 4.0 at 25C
    this->_neutralVoltage = 2.0;     //buffer solution 7.0 at 25C
    readCharacteristicValues();
}

//********************************************************************************************
// function name: setup ()
// Function Description: Initializes the ph sensor
//********************************************************************************************
void GravityPh::setup()
{
	  pinMode(phSensorPin, INPUT);

    /*EEPROM_read(PHVALUEADDR, this->_neutralVoltage);  //load the neutral (pH = 7.0)voltage of the pH board from the EEPROM
    //Serial.print("_neutralVoltage:");
    //Serial.println(this->_neutralVoltage);
    if(EEPROM.read(PHVALUEADDR)==0xFF && EEPROM.read(PHVALUEADDR+1)==0xFF && EEPROM.read(PHVALUEADDR+2)==0xFF && EEPROM.read(PHVALUEADDR+3)==0xFF){
        this->_neutralVoltage = 1500.0;  // new EEPROM, write typical voltage
        EEPROM_write(PHVALUEADDR, this->_neutralVoltage);
    }
    EEPROM_read(PHVALUEADDR+4, this->_acidVoltage);//load the acid (pH = 4.0) voltage of the pH board from the EEPROM
    //Serial.print("_acidVoltage:");
    //Serial.println(this->_acidVoltage);
    if(EEPROM.read(PHVALUEADDR+4)==0xFF && EEPROM.read(PHVALUEADDR+5)==0xFF && EEPROM.read(PHVALUEADDR+6)==0xFF && EEPROM.read(PHVALUEADDR+7)==0xFF){
        this->_acidVoltage = 2032.44;  // new EEPROM, write typical voltage
        EEPROM_write(PHVALUEADDR+4, this->_acidVoltage);
    }*/
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

		if (pHArrayIndex == arrayLength)   // 5 * 20 = 100ms
		{
			pHArrayIndex = 0;
			for (int i = 0; i < arrayLength; i++)
				this->sum += pHArray[i];
			averageVoltage = this->sum / arrayLength;
			this->sum = 0;
      //Serial.print("AvgVoltage: ");
      //Serial.println(averageVoltage);
			voltage = averageVoltage*5.0 / 1024.0;
      //Serial.print("Voltage: ");
      //Serial.println(voltage);
			pHValue = 3.5*voltage + this->offset;
		}

	}
  /*voltage = analogRead(phSensorPin)/1024.0*5000;
  //Serial.print("Voltage: ");
  //Serial.println(voltage);
  double slope = (7.0-4.0)/((this->_neutralVoltage-1500.0)/3.0 - (this->_acidVoltage-1500.0)/3.0);  // two point: (_neutralVoltage,7.0),(_acidVoltage,4.0)
  double intercept =  7.0 - slope*(this->_neutralVoltage-1500.0)/3.0;
  pHValue = slope*(voltage-1500.0)/3.0+intercept;  //y = k*x + b*/
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
    //EEPROM_read(PHVALUEADDR, this->_neutralVoltage);  //load the neutral (pH = 7.0)voltage of the pH board from the EEPROM
    //Serial.print("_neutralVoltage:");
    //Serial.println(this->_neutralVoltage);
    if(EEPROM.read(PHVALUEADDR)==0xFF && EEPROM.read(PHVALUEADDR+1)==0xFF && EEPROM.read(PHVALUEADDR+2)==0xFF && EEPROM.read(PHVALUEADDR+3)==0xFF){
        this->_neutralVoltage = 2.0;  // new EEPROM, write typical voltage
        EEPROM_write(PHVALUEADDR, this->_neutralVoltage);
    }
    //EEPROM_read(PHVALUEADDR+4, this->_acidVoltage);//load the acid (pH = 4.0) voltage of the pH board from the EEPROM
    //Serial.print("_acidVoltage:");
    //Serial.println(this->_acidVoltage);
    if(EEPROM.read(PHVALUEADDR+4)==0xFF && EEPROM.read(PHVALUEADDR+5)==0xFF && EEPROM.read(PHVALUEADDR+6)==0xFF && EEPROM.read(PHVALUEADDR+7)==0xFF){
        this->_acidVoltage = 1.14;  // new EEPROM, write typical voltage
        EEPROM_write(PHVALUEADDR+4, this->_acidVoltage);
    }
}

 /* void GravityPh::calibration(char* cmd)
{
    strupr(cmd);
    phCalibration(cmdParse(cmd));  // if received Serial CMD from the serial monitor, enter into the calibration mode
}

void GravityPh::calibration()
{
    if(cmdSerialDataAvailable() > 0)
    {
        phCalibration(cmdParse());  // if received Serial CMD from the serial monitor, enter into the calibration mode
    }
} */
void GravityPh::calibration(byte mode){
    phCalibration(mode);  // if received Serial CMD from the serial monitor, enter into the calibration mode
}

/* boolean GravityPh::cmdSerialDataAvailable()
{
    char cmdReceivedChar;
    static unsigned long cmdReceivedTimeOut = millis();
    while(Serial.available()>0){
        if(millis() - cmdReceivedTimeOut > 500U){
            this->_cmdReceivedBufferIndex = 0;
            memset(this->_cmdReceivedBuffer,0,(ReceivedBufferLength));
        }
        cmdReceivedTimeOut = millis();
        cmdReceivedChar = Serial.read();
        if (cmdReceivedChar == '\n' || this->_cmdReceivedBufferIndex==ReceivedBufferLength-1){
            this->_cmdReceivedBufferIndex = 0;
            strupr(this->_cmdReceivedBuffer);
            return true;
        }else{
            this->_cmdReceivedBuffer[this->_cmdReceivedBufferIndex] = cmdReceivedChar;
            this->_cmdReceivedBufferIndex++;
        }
    }
    return false;
} */

/* byte GravityPh::cmdParse(const char* cmd)
{
    byte modeIndex = 0;
    if(strstr(cmd, "ENTERPH")      != NULL){
        modeIndex = 1;
    }else if(strstr(cmd, "EXITPH") != NULL){
        modeIndex = 3;
    }else if(strstr(cmd, "CALPH")  != NULL){
        modeIndex = 2;
    }
    return modeIndex;
} */
/* 
byte GravityPh::cmdParse()
{
    byte modeIndex = 0;
    if(strstr(this->_cmdReceivedBuffer, "ENTERPH")      != NULL){
        modeIndex = 1;
    }else if(strstr(this->_cmdReceivedBuffer, "EXITPH") != NULL){
        modeIndex = 3;
    }else if(strstr(this->_cmdReceivedBuffer, "CALPH")  != NULL){
        modeIndex = 2;
    }
    return modeIndex;
} */

void GravityPh::phCalibration(byte mode)
{
    char *receivedBufferPtr;
    static boolean phCalibrationFinish  = 0;
    static boolean enterCalibrationFlag = 0;
    switch(mode){
        case 0:
        if(enterCalibrationFlag){
            Serial.println(F(">>>Command Error<<<"));
        }
        break;

        case 1:
        enterCalibrationFlag = 1;
        phCalibrationFinish  = 0;
        Serial.println();
        Serial.println(F(">>>Enter PH Calibration Mode<<<"));
        Serial.println(F(">>>Please put the probe into the 4.0 or 7.0 standard buffer solution<<<"));
        Serial.println();
        break;

        case 2:
        if(enterCalibrationFlag){
            if((voltage>1.7)&&(voltage<2.7)){        // buffer solution:7.0{
                Serial.println();
                Serial.print(F(">>>Buffer Solution:7.0"));
                this->_neutralVoltage =  voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println();
                phCalibrationFinish = 1;
            }else if((voltage>0.7)&&(voltage<1.7)){  //buffer solution:4.0
                Serial.println();
                Serial.print(F(">>>Buffer Solution:4.0"));
                this->_acidVoltage =  voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<")); 
                Serial.println();
                phCalibrationFinish = 1;
            }else{
                Serial.println();
                Serial.print(F(">>>Buffer Solution Error Try Again<<<"));
                Serial.println();                                    // not buffer solution or faulty operation
                phCalibrationFinish = 0;
            }
        }
        break;

        case 3:
        if(enterCalibrationFlag){
            Serial.println();
            if(phCalibrationFinish){
                if((voltage>1.7)&&(voltage<2.7)){
                    EEPROM_write(PHVALUEADDR, this->_neutralVoltage);
                }else if((voltage>0.7)&&(voltage<1.7)){
                    EEPROM_write(PHVALUEADDR+4, this->_acidVoltage);
                }
                Serial.print(F(">>>Calibration Successful"));
            }else{
                Serial.print(F(">>>Calibration Failed"));
            }
            Serial.println(F(",Exit PH Calibration Mode<<<"));
            Serial.println();
            phCalibrationFinish  = 0;
            enterCalibrationFlag = 0;
        }
        break;
    }
}
