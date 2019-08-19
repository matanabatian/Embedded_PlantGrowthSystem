#ifndef ENVCTRL_H
#define ENVCTRL_H

#include <SimpleTimer.h>
#include "Arduino.h"
#include "MUX74HC4067.h"


class EnvironmentControl
{
  public:
    // Methods
	  EnvironmentControl();
    /* Soil moisture measure & water pump methods */
	  int SoilMoistureLevel();
	  void WaterPumpControl(int minHumidity, int maxHumidity);

    /* UV light measure & UV light bulb methods */
    int UvLightLevel();
    void UvLightControl(int hoursOfLightCycle);

    // Variables
    int currentSoilHumidity;
    int currentUvLight;
    
    /* Cost Variables */ 
    // This need to be in a new h/cpp file
    int pumpWorkingTimerCounter;
    int sumOfUvLightWorkingTime;
    int pumpDailyVoltage;
    int uvBulbDailyVoltage;
    int relayDailyVoltage;
    
};

#endif
