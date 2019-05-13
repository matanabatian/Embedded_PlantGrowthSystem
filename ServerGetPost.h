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
    String newResearchURL = "/Research/GetNewResearchByIp?plantIp=10.12.156.65";
    String newIntervalsURL = "/Plant/GetIntervalsByDate?id=5c9123439ec5fc4398bfef5b";
};


#endif
