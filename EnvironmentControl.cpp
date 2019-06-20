#include "EnvironmentControl.h"

//Relay variables
const int relayPinPump = D2; // Digital pin 7 output for pump in order to toggle HIGH / LOW
const int relayPinUV = D1; // Digital pin 1 output for UV light in order to toggle HIGH / LOW

// Creates a MUX74HC4067 instance
// 1st argument is the Arduino PIN to which the EN pin connects
// 2nd-5th arguments are the Arduino PINs to which the S0-S3 pins connect
MUX74HC4067 mux(D3, D0);

EnvironmentControl::EnvironmentControl()
{
  // Configures how the SIG pin will be interfaced
  // e.g. The SIG pin connects to PIN A0 on the NodeMCU,
  //      and PIN A0 is a analog input
  mux.signalPin(A0, INPUT, ANALOG);
  
  pinMode(relayPinPump, OUTPUT);// pinMode for Pump
  pinMode(relayPinUV, OUTPUT);// pinMode for UV light

  currentSoilHumidity = -1;
  currentUvLight = -1;
}

int EnvironmentControl::SoilMoistureLevel()
{
  byte soilMoistureMuxChannel = 0;
  mux.setChannel(soilMoistureMuxChannel);
  int rawDryValue = 1024;
  int rawWetValue = 260;
  int percentageDryValue = 0;
  int percentageWetValue = 100;

  int rawValue = mux.read();
  int percentageValue = map(rawValue, rawDryValue, rawWetValue, percentageDryValue, percentageWetValue);

  currentSoilHumidity = percentageValue;
  Serial.print(percentageValue);
  Serial.println("%");

  mux.disable();
  delay(1000);
  return percentageValue;
}

void EnvironmentControl::WaterPumpControl(int minHumidity, int maxHumidity)
{
  //Serial.println(SoilMoistureLevel());
  if(SoilMoistureLevel() <= minHumidity)
  {
    while(SoilMoistureLevel() <= (minHumidity+maxHumidity)/2)
    {
      Serial.println("The current soil humidity is lower then the required level. Starting to water the plant...");
      digitalWrite(relayPinPump, HIGH);
      for (int count = 0 ; count < 3 ; count++){
        pumpWorkingTimerCounter++;
        delay(1000);
      } 
  
      /* Turn off pump and delay execution of code for more accurate measurement of soil moisture */
      digitalWrite(relayPinPump, LOW);
      delay(3000);    
    }
  }
  Serial.println("The current soil humidity level reached the required level. Stopping the water pump...");
}

int EnvironmentControl::UvLightLevel()
{
  byte uvLightMuxChannel = 1;
  mux.setChannel(uvLightMuxChannel);
  int sensorValue;
  long  sum=0;
  for(int i=0;i<1024;i++)
   {  
      sensorValue=mux.read();  
      sum=sensorValue+sum;
      delay(2);
   }   
   sum = sum >> 10;
   float Vsig = sum*4980.0/1023.0; // Vsig is the value of voltage measured from the SIG pin of the Grove interface
  if (Vsig < 50) {Serial.print("UV Index: 0 ");currentUvLight = 0;mux.disable();return 0;}
  if (Vsig > 50 && Vsig < 227) {Serial.print("UV Index: 1 ");currentUvLight = 1;mux.disable();return 1;}
  if (Vsig > 227 && Vsig < 318) {Serial.print("UV Index: 2 ");currentUvLight = 2;mux.disable();return 2;}
  if (Vsig > 318 && Vsig < 408) {Serial.print("UV Index: 3 ");currentUvLight = 3;mux.disable();return 3;}
  if (Vsig > 408 && Vsig < 503) {Serial.print("UV Index: 4 ");currentUvLight = 4;mux.disable();return 4;}
  if (Vsig > 503 && Vsig < 606) {Serial.print("UV Index: 5 ");currentUvLight = 5;mux.disable();return 5;}
  if (Vsig > 606 && Vsig < 696) {Serial.print("UV Index: 6 ");currentUvLight = 6;mux.disable();return 6;}
  if (Vsig > 696 && Vsig < 795) {Serial.print("UV Index: 7 ");currentUvLight = 7;mux.disable();return 7;}
  if (Vsig > 795 && Vsig < 881) {Serial.print("UV Index: 8 ");currentUvLight = 8;mux.disable();return 8;}
  if (Vsig > 881 && Vsig < 976) {Serial.print("UV Index: 9 ");currentUvLight = 9;mux.disable();return 9;}
  if (Vsig > 976 && Vsig < 1079) {Serial.print("UV Index: 10 ");currentUvLight = 10;mux.disable();return 10;}
  if (Vsig > 1079 && Vsig < 1170) {Serial.print("UV Index: 11 ");currentUvLight = 11;mux.disable();return 11;}
  if (Vsig > 1170) {Serial.print("UV Index: 11+ ");currentUvLight = 12;mux.disable();return 12;}

}

bool uvLightToggleFlg = true;

void EnvironmentControl::UvLightControl(int hoursOfLightCycle)
{
  if(uvLightToggleFlg == true)
  {
    digitalWrite(relayPinUV, HIGH);
    sumOfUvLightWorkingTime += hoursOfLightCycle;
    uvLightToggleFlg = false;
  }
  else
  {
    digitalWrite(relayPinUV, LOW);
    uvLightToggleFlg = true;
  }
}
