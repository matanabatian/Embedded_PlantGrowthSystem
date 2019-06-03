
#include "ServerGetPost.h"

WiFiClientSecure httpsClient;

//SHA1 fingerprint of the certificate
const char fingerprint[] PROGMEM = "3ab0b1c27f746fd90c34f0d6a960cf73a4229de8";

//Https information
const char* host = "plantgrowthsystembackend.azurewebsites.net";
const int httpsPort = 443;

ServerGetPost::ServerGetPost(){}

//HTTPS GET is used to request data from a specified resource.
String ServerGetPost::httpsGet(String url)
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

//HTTPS POST is used to send data to a server to create/update a resource.
void ServerGetPost::httpsPost(String url, String reasearchSamples)
{
  httpsClient.setFingerprint (fingerprint );
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
