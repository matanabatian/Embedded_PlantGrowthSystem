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
    String newResearchURL = "/Research/GetNewResearchByIp?plantIp=987.456.987.456&envStatus=";
    String newIntervalsURL = "/Plant/GetIntervalsByDate?id=";
    //URLs to update server that the system is working
    String systemIsWorkingURL = "/Plant/plantPreTest?plantId=";
};


#endif
