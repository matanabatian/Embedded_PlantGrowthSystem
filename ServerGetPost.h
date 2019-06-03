#ifndef SERVER_GET_POST_H
#define SERVER_GET_POST_H

#include "Arduino.h"
#include <ESP8266WiFi.h>

class ServerGetPost
{
  public:
    // Methods
    ServerGetPost();
    String httpsGet(String url);
    void httpsPost(String url, String reasearchSamples);

    // Variables
    /* Urls for functions */
    String sendSamplesURL = "/Plant/UpdateMeasure";
//    String newResearchURL = "/Research/GetNewResearchByIp?plantIp=192.168.1.12";
    String newResearchURL = "/Research/GetNewResearchByIp?plantIp=987.456.987.456";
    String newIntervalsURL = "/Plant/GetIntervalsByDate?id=";
};


#endif
