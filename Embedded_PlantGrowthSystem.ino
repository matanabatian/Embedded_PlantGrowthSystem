#include <ESP8266WiFi.h>
#include <PubSubClient.h>   
#include <stdlib.h>

//Wifi Information
const char *ssid =  "daizi";   // WiFi username
const char *pass =  "leon1992";   //WiFi password

//MQTT Information
#define MQTT_SERVER "m24.cloudmqtt.com"
#define MQTT_USER "fcayusle"
#define MQTT_PASSWORD "qo3nLLQlEEUz"
#define MQTT_PORT 16067

WiFiClient wclient;  //Declares a WifiClient Object using ESP8266WiFi
PubSubClient client(wclient);  //instanciates client object

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

void ConnectToMQTT()
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("Connected to MQTT server!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  //client.subscribe("Bla");
}

//Function that holds the received message data
void callback(char* topic, byte *payload, unsigned int length) 
{
    Serial.println("A message has been recieved from server!");
    Serial.print("Topic:");
    Serial.println(topic);
    Serial.print("Data:");  
    Serial.write(payload, length);
    Serial.println();
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

void setup()
{
  Serial.begin(115200);
  Serial.println();
  ConnectToWifi();//Connecting to wifi
  client.setServer(MQTT_SERVER, MQTT_PORT);//Setting MQTT server
  client.setCallback(callback);//Setting the callback function
  pinMode(relayPin, OUTPUT);//pinMode for Pump
}

void loop() 
{
  if (!client.connected()) 
    ConnectToMQTT();

   if(WiFi.status() == WL_CONNECTION_LOST)
   {
    Serial.println("WiFi connection lost! Trying to reconnect...");
    ConnectToWifi();
   }

  //Listener for incoming and outgoing data
  client.loop();
  
//client.publish("temp_topic", "Bla Bla Bla", true);

  PumpControl(80);
}
