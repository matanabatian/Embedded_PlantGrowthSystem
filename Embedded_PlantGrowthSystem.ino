using namespace std;

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <stdlib.h>

#define NEW_REASEARCH_DELAY 60000
#define NEW_INTERVALS_DELAY 30000
#define SEND_SENSORS_SAMPLES_DELAY 10000

//Wifi Information
const char *ssid =  "Matan&Keren";   // WiFi username
const char *pass =  "304865215KF";   //WiFi password

//Last execution time
unsigned long checkForNewReasearch;
unsigned long checkForNewIntervals;
unsigned long sendSensorsSamples;

HTTPClient http;
WiFiClient client;

//Pump variables
const int relayPin = D7; // Digital pin 7 output for pump

/* Internet related functions */
//Setting connection to WiFi
void ConnectToWifi()
{
  WiFi.begin(ssid, pass);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi!, IP address: ");
  Serial.println(WiFi.localIP());
}

/* Sensors related functions*/
//This function returns the soil moistue level in percentage
int SoilMoistureLevel()
{
  int rawDryValue = 1023;
  int rawWetValue = 340;
  int percentageDryValue = 0;
  int percentageWetValue = 100;

  int rawValue = analogRead(A0);
  int percentageValue = map(rawValue, rawDryValue, rawWetValue, percentageDryValue, percentageWetValue);

  Serial.print(percentageValue);
  Serial.println("%");
  
  return percentageValue;
}

//This function control the pump 
bool PumpControl(int desiredSoilMoistureLevel)
{
  if(SoilMoistureLevel() < desiredSoilMoistureLevel)
  {
    digitalWrite(relayPin, HIGH);
    for (int count = 0 ; count < 5 ; count++)
      delay(1000);
  }
  else
    digitalWrite(relayPin, LOW); 
}

String HttpPost(String url, String content)
{
    //Specify request destination
    if(http.begin(url))
    {
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header
      int httpCode = http.POST(content);   //Send the request
      String payload = http.getString();    //Get the response payload
      Serial.println(httpCode);   //Print HTTP return code
      Serial.println(payload);    //Print request response payload
      http.end();  //Close connection
    }
}

String HttpGet(String url)
{
  if (http.begin(client, url)) // HTTP
  {  
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0) 
      {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) 
        {
          String payload = http.getString();
          Serial.println(payload);
          return payload;
        }
      } 
      else 
      {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        return http.errorToString(httpCode).c_str();
      }
      http.end();
    } 
    else 
    {
      Serial.printf("[HTTP} Unable to connect\n");
      return "[HTTP} Unable to connect\n";
    }
}

void NewResearch()
{
  Serial.println("New research!");
  //HttpGet("url");
}

void NewIntervals()
{
  Serial.println("New Intervals!");
  //HttpGet("url");
}

void SendSensorsSamples()
{
  Serial.println("Sensors samples!");
  //HttpPost("url", "data");
}

void ExecuteFunction(int delayTime)
{
  switch(delayTime)
  {
    case 60000: NewResearch();break;
    case 30000: NewIntervals();break;
    case 10000: SendSensorsSamples();break;
  }
}

unsigned long DelayExecution(unsigned long my_time,int delayTime)
{
  if(millis()-my_time > delayTime)     //Has one second passed?
  {
    ExecuteFunction(delayTime);
    return millis();         //and reset time.
  }
  return my_time;
}

void setup()
{
  Serial.begin(115200);
  checkForNewReasearch = sendSensorsSamples = checkForNewIntervals = millis();
  ConnectToWifi();//Connecting to wifi
//  pinMode(relayPin, OUTPUT);//pinMode for Pump
}

void loop() 
{
   if(WiFi.status() == WL_CONNECTION_LOST)
   {
      Serial.println("WiFi connection lost! Trying to reconnect...");
      ConnectToWifi();
   }
    checkForNewReasearch = DelayExecution(checkForNewReasearch, NEW_REASEARCH_DELAY);
    checkForNewIntervals = DelayExecution(checkForNewIntervals, NEW_INTERVALS_DELAY);
    sendSensorsSamples = DelayExecution(sendSensorsSamples, SEND_SENSORS_SAMPLES_DELAY);
}
