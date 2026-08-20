#include "log_buffer.h"
#include <string.h>

static const size_t LOG_CAP = 6144;
static char logBuf[LOG_CAP];
static size_t logHead = 0;
static size_t logLen = 0;

static void ringPut(char c) {
    logBuf[logHead] = c;
    logHead = (logHead + 1) % LOG_CAP;
    if (logLen < LOG_CAP) logLen++;
}

class TeePrint : public Print {
public:
    size_t write(uint8_t c) override {
        Serial.write(c);
        ringPut((char)c);
        return 1;
    }
};

static TeePrint tee;

void LogBuffer::begin() {
    logHead = 0;
    logLen = 0;
}

Print &LogBuffer::out() {
    return tee;
}

void LogBuffer::copyTo(String &out) {
    out = "";
    if (logLen == 0) return;
    out.reserve(logLen + 1);
    size_t start = (logHead + LOG_CAP - logLen) % LOG_CAP;
    for (size_t i = 0; i < logLen; i++) {
        out += logBuf[(start + i) % LOG_CAP];
    }
}
