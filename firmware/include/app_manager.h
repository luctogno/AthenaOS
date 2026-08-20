#ifndef ATHENAOS_APP_MANAGER_H
#define ATHENAOS_APP_MANAGER_H

#include "app.h"

#define APP_MANAGER_MAX 8

class AppManager {
public:
    void registerApp(App *app);
    void switchToApp(int index);
    void switchToAppById(const char *id);
    App *getCurrentApp();
    int getAppCount() const { return _count; }
    App *getApp(int index);
    int getCurrentIndex() const { return _current; }

private:
    App *_apps[APP_MANAGER_MAX] = {};
    int _count = 0;
    int _current = -1;
};

extern AppManager g_appManager;

#endif
