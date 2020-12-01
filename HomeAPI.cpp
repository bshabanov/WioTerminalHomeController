#include "HomeAPI.h"

String HomeAPI::url = "";

void HomeAPI::get(String _url) {
  url = _url;
}

void HomeAPI::getQueue() {
  if (HomeAPI::url != "") {
    HTTPClient http;
    http.begin(HomeAPI::url);
    int httpCode = http.GET();
    http.end();
    HomeAPI::url = "";
  }
}

void HomeAPI::setup(TFT_eSPI *tft) {

  static char* ssid = "";
  static char* password =  "";

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  WiFi.begin(ssid, password);

  tft->drawString("Connecting to WiFi", 10, 20);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    tft->drawString("Connecting...", 10, 30);
    delay(500);
    tft->drawString("Connecting......", 10, 30);
    Serial.println(WiFi.status());
  }
  Serial.print("Connected to the WiFi network with IP: ");
  Serial.println(WiFi.localIP());
}

void HomeAPI::tvPower() {
  get("");
}

void HomeAPI::tvVolUp() {
  get("");
}

void HomeAPI::tvVolDown() {
  get("");
}

void HomeAPI::tvMute() {
  get("");
}

void HomeAPI::tvSource() {
  get("");
}

void HomeAPI::lightKitchen(bool state) {
  get("");
}
