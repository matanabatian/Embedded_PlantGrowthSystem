using namespace std;

#include <ArduinoJson.h>
#include <stdlib.h>
#include <string> 

#include "Arduino.h"
#include "Wifi.h"
#include "EnvironmentControl.h"
#include "ServerGetPost.h"

/* Functions timer */
SimpleTimer mainTimer;

/* Timers IDs */
int newResearchTimerID; 
int newIntervalsID;
int MeasureGrowthEnvironmentID;
int SendDataToServerID;
int StartUvLightCycleID;
int StartWaterPumpID;

/* Research information */
String researchID;
int freaquencyOfMeasurement;
int frequencyOfUpload;
int minHumidity, maxHumidity;
int hoursOfLightCycle;

/* Daily values for the research */
String dailyIntervals; //jason

/* Samples values */
String researchSamples = "{'id': '5c9123439ec5fc4398bfef5b','Light': '12', 'Humidity': '26', 'WaterAmount': '15', 'PowerConsumption': '25.2342'}";

/* Create wifi instance */
WifiConnection wifi = WifiConnection(); 

/* Create environment control instance */
EnvironmentControl environmentControl = EnvironmentControl();

/* Create GET & POST instance */
ServerGetPost serverGetPost = ServerGetPost();
/*-----------------------------------------------------------------------------------------------------------------------------------------------------------*/

/*!
 * @brief: This function calculates the total power consumption per frequencyOfUpload
 * @param:
 * @return: pumpPowerConsumption               
 *          water pump power consumption total value.
 * @return: uvBulbPowerConsumption      
 *          UV bulb power consumption total value.
 */
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

/*!
 * @brief: update strings that send to DB
 * @param: 
 * @return:
 */
void UpdateSamplesString()
{
  double powerConsumption = CalculateTotalPowerConsumption();
  double waterConsumption = 0.0075 * (double)environmentControl.pumpWorkingTimerCounter;
  researchSamples = "{'id': '"+researchID+"', 'Light': '"+String(environmentControl.currentUvLight)+"', 'Humidity': '"+String(environmentControl.currentSoilHumidity)+"', 'WaterAmount': '"+String(waterConsumption, 10)+"', 'PowerConsumption': '"+String(powerConsumption)+"'  }";
}

/*!
 * @brief: Printing method for check
 * @param: 
 * @return:
 */
void PrintResearchIntervals()
{
  Serial.println(freaquencyOfMeasurement);
  Serial.println(frequencyOfUpload);
  Serial.println(minHumidity);
  Serial.println(maxHumidity);
  Serial.println(hoursOfLightCycle);
}

/*!
 * @brief: stop all research timers and set research timer 
 *         to look for new research every 10 min.
 * 
 * @param: 
 * @return:
 */
void StopResearch()
{
  mainTimer.disable(MeasureGrowthEnvironmentID);
  mainTimer.disable(SendDataToServerID);
  mainTimer.disable(StartUvLightCycleID);
  mainTimer.disable(StartWaterPumpID);
  
  newResearchTimerID = mainTimer.setInterval(10000, NewResearch); // Search for new research every 10 min 
}

/*!
 * @brief: Disassemble Json file data with research intervals and fetch values
 * @param: jsonIntervals
 *         Json file
 * @return:
 */
void DisassembleJson(String jsonIntervals)
{
  Serial.println(jsonIntervals);
  StaticJsonDocument<500> doc; //allocation
  DeserializationError error = deserializeJson(doc,jsonIntervals); // Test if parsing succeeds.
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

/*!
 * @brief: sensors check before research.
 * @param: 
 * @return: "Ok" - soil or uv sensors are not good to go
 * @return: "Eror" - soil and uv sensors are good to go
 */
String CheckSystemIntegrity()
{
  environmentControl.SoilMoistureLevel();
  environmentControl.UvLightLevel();
  if(environmentControl.currentSoilHumidity != -1 && environmentControl.currentUvLight != -1)
    return "Ok";
  else
    return "Error";
}

/*!
 * @brief:Keep running in loop until sensors will be fixed.
 * @param: 
 * @return:
 */
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

/*!
 * @brief: sensors check + find new research + set research timers.
 * @param: 
 * @return:
 */
void NewResearch()
{ 
  String systemCheckResult = CheckSystemIntegrity(); //sensors check
  researchID = serverGetPost.httpsGet(serverGetPost.newResearchURL + systemCheckResult); //geting research ID
  if(systemCheckResult == "Error")
    LoopUntilSystemIsFixed();
    
  if(researchID != NULL && researchID != "Error")
  {
    Serial.println("New research Found!");
    NewIntervals(); //set all timers by intervals
    newIntervalsID = mainTimer.setInterval(24 * 3600000, NewIntervals);// update all intervals any 24 hours
    StartWaterPumpID = mainTimer.setInterval(900000, StartWaterPump); // Starting water pump every 15 minutes(only if the current soil moisture is less then the required minimum soil moisture)
    mainTimer.disable(newResearchTimerID); //Disable newResearch function timer
  }
  else
    Serial.println("There is no new research!");
}

/*!
 * @brief: get new interval, set all research timers by the new json interval demand ("dailyIntervals").
 * @param: 
 * @return:
 */
void NewIntervals()
{
  if(MeasureGrowthEnvironmentID != NULL && SendDataToServerID != NULL && StartUvLightCycleID != NULL) //If there was research before the current one
  {
    mainTimer.disable(MeasureGrowthEnvironmentID);
    mainTimer.disable(SendDataToServerID);
    mainTimer.disable(StartUvLightCycleID);
  }

  dailyIntervals = serverGetPost.httpsGet(serverGetPost.newIntervalsURL + researchID);//geting json file
  DisassembleJson(dailyIntervals); // disassemble jason
  
  /*set timers */
  MeasureGrowthEnvironmentID = mainTimer.setInterval(freaquencyOfMeasurement * 3600000, MeasureGrowthEnvironment); //Measure growth environment every freaquencyOfMeasurement hours
  SendDataToServerID = mainTimer.setInterval(frequencyOfUpload * 3600000, SendDataToServer); //Send research samples every frequencyOfUpload hours
  StartUvLightCycleID = mainTimer.setInterval(hoursOfLightCycle * 3600000, StartUvLightCycle); // Starting UV light cycle
}

/*!
 * @brief: Measure growth environment.
 * @param: 
 * @return:
 */
void MeasureGrowthEnvironment()
{
  environmentControl.SoilMoistureLevel();
  environmentControl.UvLightLevel();
}

/*!
 * @brief:send research samples to server 
 * @param: 
 * @return:
 */
void SendDataToServer()
{
  UpdateSamplesString(); //update strings that send to DB
  serverGetPost.httpsPost(serverGetPost.sendSamplesURL, researchSamples); //send research samples
  environmentControl.pumpWorkingTimerCounter = 0; // Reset pump timer to 0 for new calc value
  environmentControl.sumOfUvLightWorkingTime = 0; // Reset UV light working time in current cycle for new calc value
}

/*!
 * @brief:
 * @param: 
 * @return:
 */
void StartWaterPump()
{
  environmentControl.WaterPumpControl(minHumidity,maxHumidity);
}

/*!
 * @brief:
 * @param: 
 * @return:
 */
void StartUvLightCycle()
{
  environmentControl.UvLightControl(hoursOfLightCycle);
}

/*!
 * @brief: function is called when a sketch starts. 
 * Use it to initialize,function will only run once, after each powerup or reset of the Arduino board.
 * 
 * @param: 
 * @return:
 */
void setup()
{
  Serial.begin(115200); // opens serial port, sets data rate to 115200 bps
  wifi.ConnectToWifi();//Connecting to wifi
  newResearchTimerID = mainTimer.setInterval(600000, NewResearch); // Search for new research every 10 min 
}

/*!
 * @brief: loops consecutively.
 * @param: 
 * @return:
 */
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
