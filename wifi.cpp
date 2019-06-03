#include "Wifi.h"

//Wifi Information
//const char *ssid =  "Shenkar-New";   // WiFi username
//const char *pass =  "Shenkarwifi";   //WiFi password
const char *ssid =  "Matan&Keren";   // WiFi username
const char *pass =  "304865215KF";   //WiFi password


WifiConnection::WifiConnection(){}

void WifiConnection::ConnectToWifi()
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
