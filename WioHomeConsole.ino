#include <Seeed_Arduino_FreeRTOS.h>
#include "GUI.cpp"
#include "HomeAPI.h"
#include"LIS3DHTR.h"

LIS3DHTR<TwoWire> lis;

void sounds(void* pvParameters) {
  bool isPressed = false;
  while (1) {
    if (digitalRead(WIO_KEY_A) == LOW || digitalRead(WIO_KEY_B) == LOW || digitalRead(WIO_KEY_C) == LOW ||
        digitalRead(WIO_5S_UP) == LOW || digitalRead(WIO_5S_DOWN) == LOW || digitalRead(WIO_5S_LEFT) == LOW ||
        digitalRead(WIO_5S_RIGHT) == LOW || digitalRead(WIO_5S_PRESS) == LOW
       ) {
      if (isPressed == false) {
        analogWrite(WIO_BUZZER, 128);
        delay(100);
        analogWrite(WIO_BUZZER, 0);
        isPressed = true;
      }
    } else {
      isPressed = false;
    }
  }
}

void apiGET(void* pvParameters) {
  while (1) {
    HomeAPI::getQueue();
  }
}

void deviceOrientation(void* pvParameters) {
  bool isLandscape = false;
  while (1) {
    float x_values, y_values;
    x_values = lis.getAccelerationX();
    y_values = lis.getAccelerationY();

    if (y_values > -0.5 &&  x_values > 0.5 && isLandscape == false) {
      Serial.println("Portrait");
      isLandscape = true;
    } else if (x_values < 0.5 && y_values < -0.5 && isLandscape == true) {
      Serial.println("Landscape");
      isLandscape = false;
    }
    delay(50);
  }
}


void setup() {
  Serial.begin(115200);
  vNopDelayMS(1000);

  // while (!Serial);
  Serial.println("");
  Serial.println("******************************");
  Serial.println("        Program start         ");
  Serial.println("******************************");
  GUI::setup();

  lis.begin(Wire1);
  lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ); //Data output rate
  lis.setFullScaleRange(LIS3DHTR_RANGE_2G);

  xTaskCreate(sounds, "Sounds ", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(apiGET, "GET API", 256, NULL, tskIDLE_PRIORITY + 1, NULL);
  xTaskCreate(deviceOrientation, "GET API", 256, NULL, tskIDLE_PRIORITY + 2, NULL);

}

void loop() {
  GUI::tick();
  delay(5);
}
