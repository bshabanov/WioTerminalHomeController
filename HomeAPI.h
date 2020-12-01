#ifndef HOMEAPI_H
#define HOMEAPI_H

#include "Arduino.h"
#include <rpcWiFi.h>
#include <TFT_eSPI.h>
#include <HTTPClient.h>

class HomeAPI {
  private:
    // Load url for API task
    void get(String _url);
  public:
    // URL to execute
    static String url;
    // Will execute url if there is one in the queue
    static void getQueue();
    // Init api * WIFI init;
    void setup(TFT_eSPI *tft);

    // TOGGLE POWER TV
    void tvPower();

    // VOLUME UP
    void tvVolUp();

    // VOLUME DOWN
    void tvVolDown();

    // MUTE TV
    void tvMute();

    // CHANGE SOURCE TV
    void tvSource();

    // KITCHEN LIGH
    void lightKitchen(bool state);
};

#endif
