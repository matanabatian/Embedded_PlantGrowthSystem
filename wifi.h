#ifndef WIFI_H
#define WIFI_H

#include "Arduino.h"
#include <ESP8266WiFi.h>

class WifiConnection
{
  public:
    WifiConnection();
    void ConnectToWifi();
};

#endif
