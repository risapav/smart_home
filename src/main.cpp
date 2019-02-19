#include <Arduino.h>
//#include <time.h>
#include <App.hpp>
//#include <WifiHandler.hpp>
#include <OtaHandler.hpp>
#include "configuration.h"

#define PIN_LED 13

#ifdef ARDUINO_OTA

#include <ArduinoOTA.h>



#endif

void setup() {
  app.setup();
  app.writeConfig();
  app.printConfig();
    // Other setup code
#ifdef ARDUINO_OTA

#endif
}

void loop() {
    // Other loop code
#ifdef ARDUINO_OTA
    otaHandler.handle();
#endif
    // Heartbeat
//    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
  Serial.print(".");
    delay(250);
}
