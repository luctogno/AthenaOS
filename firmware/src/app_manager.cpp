#include "app_manager.h"
#include "boards/board.h"
#include <string.h>

AppManager g_appManager;

void AppManager::registerApp(App *app) {
    if (!app || _count >= APP_MANAGER_MAX) return;
    AppManifest m = app->getManifest();
    if (!m.enabled) {
        DEBUG_PRINTF("[AppMgr] skip disabled %s\n", m.id ? m.id : "?");
        return;
    }
    _apps[_count++] = app;
    app->init();
    DEBUG_PRINTF("[AppMgr] registered %s (%s)\n", m.id, m.name);
}

void AppManager::switchToApp(int index) {
    if (index < 0 || index >= _count) return;
    if (index == _current) return;

    App *next = _apps[index];
    if (!next) return;

    if (_current >= 0 && _apps[_current]) {
        if (_apps[_current]->state == STATE_RUNNING) {
            _apps[_current]->pause();
        }
    }

    _current = index;
    if (next->state == STATE_PAUSED) next->resume();
    else next->start();

    AppManifest m = next->getManifest();
    DEBUG_PRINTF("[AppMgr] switch -> %s\n", m.id);
}

void AppManager::switchToAppById(const char *id) {
    if (!id) return;
    for (int i = 0; i < _count; i++) {
        AppManifest m = _apps[i]->getManifest();
        if (m.id && strcmp(m.id, id) == 0) {
            switchToApp(i);
            return;
        }
    }
    DEBUG_PRINTF("[AppMgr] app not found: %s\n", id);
}

App *AppManager::getCurrentApp() {
    if (_current < 0 || _current >= _count) return nullptr;
    return _apps[_current];
}

App *AppManager::getApp(int index) {
    if (index < 0 || index >= _count) return nullptr;
    return _apps[index];
}
