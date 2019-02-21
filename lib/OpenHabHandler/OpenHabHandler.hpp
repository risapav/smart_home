#ifndef __OPENHAB_HANDLER_H__
#define __OPENHAB_HANDLER_H__

// https://www.openhab.org/
// The open Home Automation Bus (openHAB, pronounced ˈəʊpənˈhæb)
// is an open source, technology agnostic home automation platform
// which runs as the center of your smart home!

#include <Arduino.h>

class OpenHabHandler
{
private:
  void sendValueV1( const char* itemName, float value );
  void sendValueV2( const char* itemName, float value );
  time_t intervalTimestamp = 0;

public:
  OpenHabHandler();
  void sendValue( const char* itemName, float value );
  void handle(time_t timestamp);

};

extern OpenHabHandler openHabHandler;

#endif
