#include "app_manager.h"
#include "tamafi_game.h"
#include "config.h"
#include "icon.h"

class TamafiApp : public App {
public:
    AppManifest getManifest() override {
        return {"tamafi", "TamaFi", "2.0.0", "AthenaOS", true, drawTamafiIcon, nullptr, 0, 0};
    }

    bool consumesInput() const override { return true; }

    void start() override {
        if (!_inited) {
            tamafiGameInit();
            _inited = true;
        } else {
            applyTftBrightness();
        }
        state = STATE_RUNNING;
    }

    void resume() override {
        applyTftBrightness();
        state = STATE_RUNNING;
    }

    void update() override {
        if (state != STATE_RUNNING) return;
        tamafiGameLoop();
    }

    void draw() override {}

private:
    bool _inited = false;
};

static TamafiApp tamafiApp;

void registerTamafiApp() {
    g_appManager.registerApp(&tamafiApp);
}
