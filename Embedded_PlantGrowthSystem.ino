using namespace std;

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <SimpleTimer.h>
#include <stdlib.h>
#include <string> 

//Functions timer
SimpleTimer functionsTimer;
//Timers IDs
int newResearchTimerID;
int newIntervalsTimerID;
int sendSamplesTimerID;

//Research information
String researchID = "5c9123439ec5fc4398bfef5b";
int freaquencyOfMeasurement;
int frequencyOfUpload;
int minHumidity, maxHumidity;
int minTemperature, maxTemperature;
int minLight, maxLight;
bool IsResearchBegin = false;

//Samples values
int soilHumidity;
int temperature;
int light;
String dailyIntervals;

//Wifi Information
const char *ssid =  "Matan&Keren";   // WiFi username
const char *pass =  "304865215KF";   //WiFi password

//Https information
const char* host = "plantgrowthsystembackend.azurewebsites.net";
const int httpsPort = 443;

//SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "3ab0b1c27f746fd90c34f0d6a960cf73a4229de8";

//Pump variables
const int relayPin = D7; // Digital pin 7 output for pump

//Urls for functions
String sendSamplesURL = "/Plant/UpdateMeasure";
String newResearchURL = "/Research/GetNewResearchByIp?plantIp=10.12.156.65";
String newIntervalsURL = "/Plant/GetIntervalsByDate?id=5c9123439ec5fc4398bfef5b";

//Data format sent to the server
String reasearchSamples = "{'id': '5c9123439ec5fc4398bfef5b' ,'Temp': '23', 'Light': '12','Humidity': '26' }";

// Use WiFiClientSecure class to create TLS connection
WiFiClientSecure httpsClient;


/* Internet related functions */
//Setting connection to WiFi
void ConnectToWifi()
{
  WiFi.mode(WIFI_STA);
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

//HTTPS POST is used to send data to a server to create/update a resource.
void HttpsPost(String url)
{  
  httpsClient.setFingerprint(fingerprint);
  if (!httpsClient.connect(host, httpsPort))
  {
    Serial.println("connection failed");
    return;
  }

  httpsClient.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Content-Type: application/json"+ "\r\n" +
               "Content-Length: "+ String(reasearchSamples.length()) + "\r\n\r\n" +
               reasearchSamples + "\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  
   while (httpsClient.connected()) 
   {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  while(httpsClient.available())
  {        
    String line = httpsClient.readStringUntil('\n');  //Read Line by Line
    Serial.println(line); //Print response
  }
  Serial.println("closing connection");
}

//HTTPS GET is used to request data from a specified resource.
String HttpsGet(String url)
{
  httpsClient.setFingerprint(fingerprint);
  if (!httpsClient.connect(host, httpsPort)) 
  {
    Serial.println("connection failed");
    return "Error";
  }

  httpsClient.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (httpsClient.connected()) 
  {
    String data = httpsClient.readStringUntil('\n');
    if (data == "\r") {
      Serial.println("headers received: ");
      Serial.println(data);
      break;
    }
  }
  String data = httpsClient.readStringUntil('\n');

  Serial.println(data);
  Serial.println("closing connection");
  return data;
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
bool WaterPumpControl(int desiredSoilMoistureLevel)
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

void UpdateSamplesString()
{
  reasearchSamples = "{'id': '5c9123439ec5fc4398bfef5b' ,'Temp': '"+String(random(20, 26))+"', 'Light': '"+String(random(0, 101))+"','Humidity': '"+String(random(10, 35))+"' }";
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
  int freaquencyOfMeasurement = doc["Frequency_of_measurement"];
  int frequencyOfUpload = doc["Frequency_of_upload"];
  int minHumidity = doc["Control_plan"]["min_Humidity"];
  int maxHumidity = doc["Control_plan"]["max_Humidity"];
  float minTemperature = doc["Control_plan"]["min_Temperature"];
  float maxTemperature = doc["Control_plan"]["max_Temperature"];
  int minLight = doc["Control_plan"]["min_Light"];
  int maxLight = doc["Control_plan"]["max_Light"];
}

void NewResearch()
{
  String tmpID;
  Serial.println("New research!");
  tmpID = HttpsGet(newResearchURL);
  if(tmpID != NULL && tmpID != "Error")
    researchID = tmpID; 
  if(researchID.IsEmpty() == false)
  {
    IsResearchBegin = true;
    NewIntervals();
    functionsTimer.disable(newResearchTimerID); //Disable newResearch function timer
  }   
}

void NewIntervals()
{
  Serial.println("New Intervals!");
  dailyIntervals = HttpsGet(newIntervalsURL);
  DisassembleJson(dailyIntervals);
  newIntervalsTimerID = functionsTimer.setInterval(freaquencyOfMeasurement * 3600000, NewIntervals); //Get research intervals every freaquencyOfMeasurement hours
  sendSamplesTimerID = functionsTimer.setInterval(frequencyOfUpload * 3600000, SendSensorsSamples); //Send research samples every frequencyOfUpload hours
}

void SendSensorsSamples()
{
  Serial.println("Sensors samples!");
  //WaterPumpControl(minHumidity);
  UpdateSamplesString();
  HttpsPost(sendSamplesURL);
}

void setup()
{
  Serial.begin(115200);
  ConnectToWifi();//Connecting to wifi
//  pinMode(relayPin, OUTPUT);//pinMode for Pump
  newResearchTimerID = functionsTimer.setInterval(600000, NewResearch);// Search for new research every 10 min
}

void loop() 
{ 
   if(WiFi.status() == WL_CONNECTION_LOST)
   {
      Serial.println("WiFi connection lost! Trying to reconnect...");
      ConnectToWifi();
   }
   functionsTimer.run();
}
