#ifndef ENVCTRL_H
#define ENVCTRL_H

#include "Arduino.h"

class EnvironmentControl
{
  public:
    // Methods
	  EnvironmentControl();
	  int SoilMoistureLevel();
	  bool WaterPumpControl(int desiredSoilMoistureLevel);

    // Variables
    int currentSoilHumidity;

    
  
};

#endif
