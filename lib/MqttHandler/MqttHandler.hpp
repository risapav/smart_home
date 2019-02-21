#ifndef __MQTT_HANDLER_H__
#define __MQTT_HANDLER_H__

// http://mqtt.org/
// MQTT is a machine-to-machine (M2M)/"Internet of Things" connectivity protocol.

#include <Arduino.h>

class MqttHandler {
private:
  bool initialized;
  bool reconnect();
  void setup();
  time_t intervalTimestamp = 0;

public:
  unsigned long reconnectCounter;

  MqttHandler();
  void handle(time_t timestamp);
  void sendValue( const char* value );
};

extern MqttHandler mqttHandler;

#endif
