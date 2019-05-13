#include "EnvironmentControl.h"

//Relay variables
const int relayPinPump = D7; // Digital pin 7 output for pump toggle HIGH / LOW

EnvironmentControl::EnvironmentControl()
{
  pinMode(relayPinPump, OUTPUT);//pinMode for Pump
}

int EnvironmentControl::SoilMoistureLevel()
{
	int rawDryValue = 1023;
	int rawWetValue = 230;
	int percentageDryValue = 0;
	int percentageWetValue = 100;

	int rawValue = analogRead(A0);
	int percentageValue = map(rawValue, rawDryValue, rawWetValue, percentageDryValue, percentageWetValue);

	currentSoilHumidity = percentageValue;
	Serial.print(percentageValue);
	Serial.println("%");

	return percentageValue;
}

bool EnvironmentControl::WaterPumpControl(int desiredSoilMoistureLevel)
{
	Serial.println(desiredSoilMoistureLevel);
	while(SoilMoistureLevel() < desiredSoilMoistureLevel)
	{
		Serial.println("The current soil humidity is lower then the required level. Starting to water the plant...");
		digitalWrite(relayPinPump, HIGH);
		for (int count = 0 ; count < 5 ; count++)
		  delay(1000);
	}
	Serial.println("The current soil humidity level is the same as the required level. Stopping the water pump...");
	digitalWrite(relayPinPump, LOW); 
}
