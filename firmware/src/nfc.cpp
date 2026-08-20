#include "nfc.h"

bool Nfc::_ready = false;

#if HAS_NFC

bool Nfc::begin() {
    DEBUG_PRINTF("[Nfc] stub PN532 @ 0x%02X IRQ=%d RST=%d (no driver yet)\n",
                 NFC_I2C_ADDR, NFC_PIN_IRQ, NFC_PIN_RST);
    _ready = false;
    return false;
}

#else

bool Nfc::begin() {
    DEBUG_PRINTLN("[Nfc] HAS_NFC=0");
    _ready = false;
    return false;
}

#endif
