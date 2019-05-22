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
int newIntervalsTimerID;
int sendSamplesTimerID;

//Research information
//String researchID = "5c9123439ec5fc4398bfef5b";
String researchID;
int freaquencyOfMeasurement;
int frequencyOfUpload;
int minHumidity, maxHumidity;
float minTemperature, maxTemperature;
int hoursOfLightCycle;


// Daily values for the research
String dailyIntervals;

// Samples values
String researchSamples = "{'id': '5c9123439ec5fc4398bfef5b', 'Temp': '23', 'Light': '12', 'Humidity': '26', 'WaterAmount': '15', 'PowerConsumption': '25'}";


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
  int pumpWorkingTimeInHours = environmentControl.pumpWorkingTimerCounter / 3600;
  int pumpPowerConsumption = pumpWorkingTimeInHours * 0.4;

  // Calculate UV bulb power consumption
  int uvBulbPowerConsumption = environmentControl.sumOfUvLightWorkingTime * 0.024;

  // Return total power consumption
  return pumpPowerConsumption + uvBulbPowerConsumption;
}

void UpdateSamplesString()
{
  double powerConsumption = CalculateTotalPowerConsumption();
  researchSamples = "{'id': '5c9123439ec5fc4398bfef5b' ,'Temp': '"+String(random(20, 26))+"', 'Light': '"+String(environmentControl.currentUvLight)+"','Humidity': '"+String(environmentControl.currentSoilHumidity)+"', 'WaterAmount': '"+String(0.0000075 * environmentControl.pumpWorkingTimerCounter)+"', 'PowerConsumption': '"+String(powerConsumption)+"'  }";
}

void PrintResearchIntervals()
{
  Serial.println(freaquencyOfMeasurement);
  Serial.println(frequencyOfUpload);
  Serial.println(minHumidity);
  Serial.println(maxHumidity);
  Serial.println(minTemperature);
  Serial.println(maxTemperature);
  Serial.println(hoursOfLightCycle);
}

void DisassembleJson(String jsonIntervals)
{
  StaticJsonDocument<320> doc;
  DeserializationError error = deserializeJson(doc,jsonIntervals);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }
  
  // Fetch values.
  freaquencyOfMeasurement = doc["Frequency_of_measurement"];
  frequencyOfUpload = doc["Frequency_of_upload"];
  minHumidity = doc["Control_plan"]["min_Humidity"];
  maxHumidity = doc["Control_plan"]["max_Humidity"];
  minTemperature = doc["Control_plan"]["min_Temperature"];
  maxTemperature = doc["Control_plan"]["max_Temperature"];
  hoursOfLightCycle = doc["Control_plan"]["light_Per_Day"];

  PrintResearchIntervals();
}

void NewResearch()
{
  researchID = serverGetPost.httpsGet(serverGetPost.newResearchURL);
  if(researchID != NULL && researchID != "Error")
  {
    Serial.println("New research Found!");
    NewIntervals();
    mainTimer.disable(newResearchTimerID); //Disable newResearch function timer
  }
  else
    Serial.println("There is no new research!");
}

void NewIntervals()
{
  //Delete old timers
  if(newIntervalsTimerID != NULL && sendSamplesTimerID != NULL)
  {
    mainTimer.disable(newIntervalsTimerID);
    mainTimer.disable(sendSamplesTimerID);
  }
  
  dailyIntervals = serverGetPost.httpsGet(serverGetPost.newIntervalsURL);
  DisassembleJson(dailyIntervals);
  newIntervalsTimerID = mainTimer.setInterval(freaquencyOfMeasurement * 3600000, NewIntervals); //Get research intervals every freaquencyOfMeasurement hours
  sendSamplesTimerID = mainTimer.setInterval(frequencyOfUpload * 3600000, SendSensorsSamples); //Send research samples every frequencyOfUpload hours
}

void SendSensorsSamples()
{
  environmentControl.WaterPumpControl(minHumidity);
  UpdateSamplesString();
  serverGetPost.httpsPost(serverGetPost.sendSamplesURL, researchSamples);
  environmentControl.pumpWorkingTimerCounter = 0; // Reset pump timer to 0
  environmentControl.sumOfUvLightWorkingTime = 0; // Reset UV light working time in current cycle
}

void startUvLightCycle()
{
  environmentControl.UvLightControl(hoursOfLightCycle);
}

void setup()
{
  Serial.begin(115200);
  wifi.ConnectToWifi();//Connecting to wifi
  //newResearchTimerID = mainTimer.setInterval(600000, NewResearch);// Search for new research every 10 min
  mainTimer.setInterval(10000,startUvLightCycle);
}

void loop() 
{ 
   if(WiFi.status() == WL_CONNECTION_LOST)
   {
      Serial.println("WiFi connection lost! Trying to reconnect...");
      wifi.ConnectToWifi();
   }
   
   mainTimer.run();
}
