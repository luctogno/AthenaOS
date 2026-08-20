#ifndef ATHENAOS_LOG_BUFFER_H
#define ATHENAOS_LOG_BUFFER_H

#include <Arduino.h>

class LogBuffer {
public:
    static void begin();
    static Print &out();
    static void copyTo(String &out);
};

#endif
