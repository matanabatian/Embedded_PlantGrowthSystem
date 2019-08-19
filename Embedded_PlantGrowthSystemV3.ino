using namespace std;

#include <ArduinoJson.h>
#include <stdlib.h>
#include <string> 

#include "Arduino.h"
#include "Wifi.h"
#include "EnvironmentControl.h"
#include "ServerGetPost.h"

//Functions timer
SimpleTimer mainTimer;

//Timers IDs
int newResearchTimerID;
int newIntervalsID;
int MeasureGrowthEnvironmentID;
int SendDataToServerID;
int StartUvLightCycleID;
int StartWaterPumpID;

//Research information
String researchID;
int freaquencyOfMeasurement;
int frequencyOfUpload;
int minHumidity, maxHumidity;
int hoursOfLightCycle;

// Daily values for the research
String dailyIntervals;

// Samples values
String researchSamples = "{'id': '5c9123439ec5fc4398bfef5b','Light': '12', 'Humidity': '26', 'WaterAmount': '15', 'PowerConsumption': '25.2342'}";


// Create wifi instance
WifiConnection wifi = WifiConnection(); 

// Create environment control instance
EnvironmentControl environmentControl = EnvironmentControl();

// Create GET & POST instance
ServerGetPost serverGetPost = ServerGetPost();

// This function calculates the total power consumption per frequencyOfUpload
double CalculateTotalPowerConsumption()
{
  // Calulcate pump power consumption
  double pumpWorkingTimeInHours = environmentControl.pumpWorkingTimerCounter / 3600;
  double pumpPowerConsumption = pumpWorkingTimeInHours * 0.005;

  // Calculate UV bulb power consumption
  double uvBulbPowerConsumption = environmentControl.sumOfUvLightWorkingTime * 0.0024; // (Volts x Amps)=  Volts x Amps= Watts DC units,then / by 1000 for kilowatt - (5 x 0.480) / 1000  =  0.0024
  // Return total power consumption
  return pumpPowerConsumption + uvBulbPowerConsumption;
}

void UpdateSamplesString()
{
  double powerConsumption = CalculateTotalPowerConsumption();
  double waterConsumption = 0.0075 * (double)environmentControl.pumpWorkingTimerCounter;
  researchSamples = "{'id': '"+researchID+"', 'Light': '"+String(environmentControl.currentUvLight)+"', 'Humidity': '"+String(environmentControl.currentSoilHumidity)+"', 'WaterAmount': '"+String(waterConsumption, 10)+"', 'PowerConsumption': '"+String(powerConsumption)+"'  }";
}

void PrintResearchIntervals()
{
  Serial.println(freaquencyOfMeasurement);
  Serial.println(frequencyOfUpload);
  Serial.println(minHumidity);
  Serial.println(maxHumidity);
  Serial.println(hoursOfLightCycle);
}

void StopResearch()
{
  mainTimer.disable(MeasureGrowthEnvironmentID);
  mainTimer.disable(SendDataToServerID);
  mainTimer.disable(StartUvLightCycleID);
  mainTimer.disable(StartWaterPumpID);
  
  newResearchTimerID = mainTimer.setInterval(10000, NewResearch); // Search for new research every 10 min 
}

void DisassembleJson(String jsonIntervals)
{
  Serial.println(jsonIntervals);
  StaticJsonDocument<500> doc;
  DeserializationError error = deserializeJson(doc,jsonIntervals);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  // Fetch values
  freaquencyOfMeasurement = doc["Frequency_of_measurement"];
  frequencyOfUpload = doc["Frequency_of_upload"];
  minHumidity = doc["Control_plan"]["min_Humidity"];
  maxHumidity = doc["Control_plan"]["max_Humidity"];
  hoursOfLightCycle = doc["Control_plan"]["light_cycle"];
  PrintResearchIntervals();
}

String CheckSystemIntegrity()
{
  environmentControl.SoilMoistureLevel();
  environmentControl.UvLightLevel();
  if(environmentControl.currentSoilHumidity != -1 && environmentControl.currentUvLight != -1)
    return "Ok";
  else
    return "Error";
}

void LoopUntilSystemIsFixed()
{
  while(true)
  {
    if(CheckSystemIntegrity() == "Ok")
    {
      serverGetPost.httpsGet(serverGetPost.systemIsWorkingURL + researchID + "&status=Ok");
      return;
    } 
    delay(5000);
  }
}

void NewResearch()
{ 
  String systemCheckResult = CheckSystemIntegrity();
  researchID = serverGetPost.httpsGet(serverGetPost.newResearchURL + systemCheckResult);
  if(systemCheckResult == "Error")
    LoopUntilSystemIsFixed();
    
  if(researchID != NULL && researchID != "Error")
  {
    Serial.println("New research Found!");
    NewIntervals();
    newIntervalsID = mainTimer.setInterval(24 * 3600000, NewIntervals);
    StartWaterPumpID = mainTimer.setInterval(900000, StartWaterPump); // Starting water pump every 15 minutes(only if the current soil moisture is less then the required minimum soil moisture)
    mainTimer.disable(newResearchTimerID); //Disable newResearch function timer
  }
  else
    Serial.println("There is no new research!");
}

void NewIntervals()
{
  //Delete old timers
  if(MeasureGrowthEnvironmentID != NULL && SendDataToServerID != NULL && StartUvLightCycleID != NULL)
  {
    mainTimer.disable(MeasureGrowthEnvironmentID);
    mainTimer.disable(SendDataToServerID);
    mainTimer.disable(StartUvLightCycleID);
  }

  dailyIntervals = serverGetPost.httpsGet(serverGetPost.newIntervalsURL + researchID);
  DisassembleJson(dailyIntervals);
  MeasureGrowthEnvironmentID = mainTimer.setInterval(freaquencyOfMeasurement * 3600000, MeasureGrowthEnvironment); //Measure growth environment every freaquencyOfMeasurement hours
  SendDataToServerID = mainTimer.setInterval(frequencyOfUpload * 3600000, SendDataToServer); //Send research samples every frequencyOfUpload hours
  StartUvLightCycleID = mainTimer.setInterval(hoursOfLightCycle * 3600000, StartUvLightCycle); // Starting UV light cycle
}

void MeasureGrowthEnvironment()
{
  environmentControl.SoilMoistureLevel();
  environmentControl.UvLightLevel();
}

void SendDataToServer()
{
  UpdateSamplesString();
  serverGetPost.httpsPost(serverGetPost.sendSamplesURL, researchSamples);
  environmentControl.pumpWorkingTimerCounter = 0; // Reset pump timer to 0
  environmentControl.sumOfUvLightWorkingTime = 0; // Reset UV light working time in current cycle
}

void StartWaterPump()
{
  environmentControl.WaterPumpControl(minHumidity,maxHumidity);
}

void StartUvLightCycle()
{
  environmentControl.UvLightControl(hoursOfLightCycle);
}

void setup()
{
  Serial.begin(115200);
  
  wifi.ConnectToWifi();//Connecting to wifi
  newResearchTimerID = mainTimer.setInterval(10000, NewResearch); // Search for new research every 10 min 
}

void loop() 
{ 
   // Reconnect if wifi connection is lost
   if(WiFi.status() == WL_CONNECTION_LOST)
   {
      Serial.println("WiFi connection lost! Trying to reconnect...");
      wifi.ConnectToWifi();
   }

   mainTimer.run();
}
