using namespace std;

#include <ArduinoJson.h>
#include <SimpleTimer.h>
#include <stdlib.h>
#include <string> 

#include "Arduino.h"
#include "Wifi.h"
#include "EnvironmentControl.h"
#include "ServerGetPost.h"

//Functions timer
SimpleTimer functionsTimer;

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
int minLight, maxLight;

// Daily values for the research
String dailyIntervals;

// Samples values
String researchSamples = "{'id': '5c9123439ec5fc4398bfef5b' ,'Temp': '23', 'Light': '12','Humidity': '26' }";


// Create wifi instance
WifiConnection wifi = WifiConnection();

// Create environment control instance
EnvironmentControl environmentControl = EnvironmentControl();

// Create GET & POST instance
ServerGetPost serverGetPost = ServerGetPost();

void UpdateSamplesString()
{
  researchSamples = "{'id': '5c9123439ec5fc4398bfef5b' ,'Temp': '"+String(random(20, 26))+"', 'Light': '"+String(random(0, 101))+"','Humidity': '"+String(environmentControl.currentSoilHumidity)+"' }";
}

void PrintResearchIntervals()
{
  Serial.println(freaquencyOfMeasurement);
  Serial.println(frequencyOfUpload);
  Serial.println(minHumidity);
  Serial.println(maxHumidity);
  Serial.println(minTemperature);
  Serial.println(maxTemperature);
  Serial.println(minLight);
  Serial.println(maxLight);
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
  minLight = doc["Control_plan"]["min_Light"];
  maxLight = doc["Control_plan"]["max_Light"];

  PrintResearchIntervals();
}

void NewResearch()
{
  researchID = serverGetPost.httpsGet(serverGetPost.newResearchURL);
  if(researchID != NULL && researchID != "Error")
  {
    Serial.println("New research Found!");
    NewIntervals();
    functionsTimer.disable(newResearchTimerID); //Disable newResearch function timer
  }
  else
    Serial.println("There is no new research!");
}

void NewIntervals()
{
  //Delete old timers
  if(newIntervalsTimerID != NULL && sendSamplesTimerID != NULL)
  {
    functionsTimer.disable(newIntervalsTimerID);
    functionsTimer.disable(sendSamplesTimerID);
  }
  
  dailyIntervals = serverGetPost.httpsGet(serverGetPost.newIntervalsURL);
  DisassembleJson(dailyIntervals);
  newIntervalsTimerID = functionsTimer.setInterval(freaquencyOfMeasurement * 3600000, NewIntervals); //Get research intervals every freaquencyOfMeasurement hours
  sendSamplesTimerID = functionsTimer.setInterval(frequencyOfUpload * 3600000, SendSensorsSamples); //Send research samples every frequencyOfUpload hours
}

void SendSensorsSamples()
{
  environmentControl.WaterPumpControl(minHumidity);
  UpdateSamplesString();
  serverGetPost.httpsPost(serverGetPost.sendSamplesURL, researchSamples);
}

void setup()
{
  Serial.begin(115200);
  wifi.ConnectToWifi();//Connecting to wifi
  newResearchTimerID = functionsTimer.setInterval(30000, NewResearch);// Search for new research every 10 min
}

void loop() 
{ 
   if(WiFi.status() == WL_CONNECTION_LOST)
   {
      Serial.println("WiFi connection lost! Trying to reconnect...");
      wifi.ConnectToWifi();
   }
   
   functionsTimer.run();
}
