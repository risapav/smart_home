#ifndef __LOCAL_TIME_HANDLER_H__
#define __LOCAL_TIME_HANDLER_H__

// to precise adjust local timer

class LocalTimeHandler {
private:
  char timeBuffer[64];
  bool initialized;

public:
  LocalTimeHandler();
  void handle();
  time_t getLocalTime();
  char *getLocalTimeText();
};

extern LocalTimeHandler localTimeHandler;

#endif
