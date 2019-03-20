#include <ESP8266WiFi.h>
#include <PubSubClient.h>   // Read the rest of the article
#include <stdlib.h>

//Wifi Information
const char *ssid =  "Matan&Keren";   // cannot be longer than 32 characters!
const char *pass =  "304865215KF";   //

//MQTT Information
#define MQTT_SERVER "m24.cloudmqtt.com"
#define MQTT_USER "fcayusle"
#define MQTT_PASSWORD "qo3nLLQlEEUz"
#define MQTT_PORT 16067

WiFiClient wclient;  //Declares a WifiClient Object using ESP8266WiFi
PubSubClient client(wclient);  //instanciates client object

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

void setup()
{
  Serial.begin(115200);
  Serial.println();
  ConnectToWifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
}

void loop() 
{
  if (!client.connected()) 
    ConnectToMQTT();

  //Listener for incoming and outgoing data
  client.loop();
  
    //client.publish("temp_topic", "Bla Bla Bla", true);
}
