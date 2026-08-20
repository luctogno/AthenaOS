#ifndef ATHENAOS_APP_H
#define ATHENAOS_APP_H

#include <Arduino.h>

enum AppState {
    STATE_IDLE = 0,
    STATE_RUNNING,
    STATE_PAUSED
};

// Launcher icon: put drawIcon in apps/<id>/icon.cpp, or a RGB565 bitmap.
typedef void (*AppIconDraw)(int16_t cx, int16_t cy, int16_t r);

struct AppManifest {
    const char *id;
    const char *name;
    const char *version;
    const char *author;
    bool enabled;
    AppIconDraw drawIcon;
    const uint16_t *iconBitmap;
    int16_t iconW;
    int16_t iconH;
};

class App {
public:
    virtual ~App() {}
    virtual AppManifest getManifest() = 0;
    virtual void init() {}
    virtual void start() { state = STATE_RUNNING; }
    virtual void update() {}
    virtual void draw() {}
    virtual void pause() { state = STATE_PAUSED; }
    virtual void resume() { state = STATE_RUNNING; }
    virtual void stop() { state = STATE_IDLE; }
    virtual void onTouchDown(uint16_t x, uint16_t y) { (void)x; (void)y; }
    virtual void onTouchUp(uint16_t x, uint16_t y) { (void)x; (void)y; }
    virtual void onButton(int button) { (void)button; }
    virtual bool consumesInput() const { return false; }

    AppState state = STATE_IDLE;
};

#endif
