/***************************************************
 DFRobot Gravity: Analog TDS Sensor/Meter
 <https://www.dfrobot.com/wiki/index.php/Gravity:_Analog_TDS_Sensor_/_Meter_For_Arduino_SKU:_SEN0244>
 
 ***************************************************
 This sample code shows how to read the tds value and calibrate it with the standard buffer solution.
 707ppm(1413us/cm)@25^c standard buffer solution is recommended.
 
 Created 2018-1-3
 By Jason <jason.ling@dfrobot.com@dfrobot.com>
 
 GNU Lesser General Public License.
 See <http://www.gnu.org/licenses/> for details.
 All above must be included in any redistribution.
 ****************************************************/

#include "GravityTDS.h"
#include "Arduino.h"
#include <EEPROM.h>
// #define TdsFactor 0.5 // tds = ec / 2
#define EEPROM_write(address, p)        \
  {                                     \
    int i = 0;                          \
    byte *pp = (byte *)&(p);            \
    for (; i < sizeof(p); i++)          \
      EEPROM.write(address + i, pp[i]); \
  }
#define EEPROM_read(address, p)         \
  {                                     \
    int i = 0;                          \
    byte *pp = (byte *)&(p);            \
    for (; i < sizeof(p); i++)          \
      pp[i] = EEPROM.read(address + i); \
  }

GravityTDS::GravityTDS(ISensor *temp) //: pin(A5),  aref(5.0), adcRange(1024.0), kValueAddress(8), kValue(1.0)
{
  this->ecTemperature = temp;
  this->pin = A1;
  // this->temperature = 25.0;
  this->aref = 5.0;
  this->adcRange = 1024.0;
  // this->kValueAddress = 8;
  this->kValueAddress = 16;
  this->kValue = 1.0;
}

GravityTDS::~GravityTDS() {}

void GravityTDS::setup()
{
  pinMode(pin, INPUT);
  readKValues();
}

float GravityTDS::getKvalue()
{
  return this->kValue;
}

void GravityTDS::update()
{
  analogValue = analogRead(pin);
  voltage = analogValue / adcRange * aref;
  ecValue = (133.42 * voltage * voltage * voltage - 255.86 * voltage * voltage + 857.39 * voltage) * kValue;
  ecValue25 = ecValue / (1.0 + 0.02 * (this->ecTemperature->getValue() - 25.0)); //temperature compensation
  tdsValue = ecValue25 * 0.5;
}

double GravityTDS::getValue()
{
  return tdsValue;
}

float GravityTDS::getEcValue()
{
  return ecValue25;
}

void GravityTDS::readKValues()
{
  EEPROM_read(this->kValueAddress, this->kValue);
  if (EEPROM.read(this->kValueAddress) == 0xFF && EEPROM.read(this->kValueAddress + 1) == 0xFF && EEPROM.read(this->kValueAddress + 2) == 0xFF && EEPROM.read(this->kValueAddress + 3) == 0xFF)
  {
    this->kValue = 1.0; // default value: K = 1.0
    EEPROM_write(this->kValueAddress, this->kValue);
  }
}

void GravityTDS::calibration(byte mode)
{
  ecCalibration(mode);
}

void GravityTDS::ecCalibration(byte mode)
{
  // char *cmdReceivedBufferPtr;
  static boolean ecCalibrationFinish = 0;
  static boolean enterCalibrationFlag = 0;
  float KValueTemp, rawECsolution;
  switch (mode)
  {
  case 0:
    if (enterCalibrationFlag)
      Serial.println(F("TDS Command Error"));
    break;

  case 1:
    enterCalibrationFlag = 1;
    ecCalibrationFinish = 0;
    Serial.println();
    Serial.println(F(">>>Enter TDS Calibration Mode<<<"));
    Serial.println(F(">>>Please put the probe into the standard buffer solution : 707ppm(1413us/cm)@25^c <<<"));
    Serial.println();
    break;

  case 2:
    //cmdReceivedBufferPtr=strstr(cmdReceivedBuffer, "CALTDS");
    //cmdReceivedBufferPtr+=strlen("CALTDS");
    // rawECsolution = /*strtod(cmdReceivedBufferPtr,NULL)*/707/(float)(0.5);
    rawECsolution = 707 / (float)(0.5);
    rawECsolution = rawECsolution * (1.0 + 0.02 * (this->ecTemperature->getValue() - 25.0));
    if (enterCalibrationFlag)
    {
      KValueTemp = rawECsolution / (133.42 * voltage * voltage * voltage - 255.86 * voltage * voltage + 857.39 * voltage); //calibrate in the  buffer solution, such as 707ppm(1413us/cm)@25^c
      if ((rawECsolution > 0) && (rawECsolution < 2000) && (KValueTemp > 0.25) && (KValueTemp < 4.0))
      {
        Serial.println();
        Serial.print(F(">>>TDS Confirm Successful,K:"));
        Serial.print(KValueTemp);
        Serial.println(F(", Send EXITTDS to Save and Exit<<<"));
        kValue = KValueTemp;
        ecCalibrationFinish = 1;
      }
      else
      {
        Serial.println();
        Serial.println(F(">>>TDS Confirm Failed,Try Again<<<"));
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
        EEPROM_write(kValueAddress, kValue);
        Serial.print(F(">>>TDS Calibration Successful,K Value Saved"));
      }
      else
        Serial.print(F(">>>TDS Calibration Failed"));
      Serial.println(F(",Exit TDS Calibration Mode<<<"));
      Serial.println();
      ecCalibrationFinish = 0;
      enterCalibrationFlag = 0;
    }
    break;
  }
}
