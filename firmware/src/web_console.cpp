#include "web_console.h"
#include "web_embed.h"
#include "boards/board.h"
#include "app_manager.h"
#include "settings.h"
#include "power.h"
#include "log_buffer.h"

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <string.h>
#include <stdio.h>

static WebServer console(WEB_CONSOLE_PORT);
static WebServer portal80(80);
static DNSServer dns;
static bool started = false;
static bool portalOn = false;

static const char *stateName(AppState s) {
    switch (s) {
        case STATE_RUNNING: return "running";
        case STATE_PAUSED:  return "paused";
        default:            return "idle";
    }
}

static void jsonEscapeAppend(String &out, const char *s) {
    if (!s) return;
    for (const char *p = s; *p; p++) {
        if (*p == '"' || *p == '\\') { out += '\\'; out += *p; }
        else if ((uint8_t)*p >= 32) out += *p;
    }
}

static void sendJson(int code, const String &body) {
    console.sendHeader("Cache-Control", "no-store");
    console.send(code, "application/json", body);
}

static String setupUrl() {
    String u = "http://";
    u += WiFi.softAPIP().toString();
    u += ":";
    u += WEB_CONSOLE_PORT;
    u += "/setup";
    return u;
}

static String statusJson() {
    bool sta = WiFi.status() == WL_CONNECTED;
    String j;
    j.reserve(1600);
    j += "{\"system\":{\"board\":\"";
    jsonEscapeAppend(j, BOARD_NAME);
    j += "\",\"v2\":";
    j += WAVESHARE_AMOLED_V2;
    j += ",\"width\":";
    j += SCREEN_WIDTH;
    j += ",\"height\":";
    j += SCREEN_HEIGHT;
    j += ",\"uptimeMs\":";
    j += millis();
    j += ",\"heap\":";
    j += ESP.getFreeHeap();
    j += ",\"ip\":\"";
    j += sta ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    j += "\",\"currentApp\":\"";
    App *cur = g_appManager.getCurrentApp();
    if (cur) jsonEscapeAppend(j, cur->getManifest().id);
    j += "\",\"wifi\":{\"connected\":";
    j += sta ? "true" : "false";
    j += ",\"ssid\":\"";
    if (sta) {
        String ssid = WiFi.SSID();
        jsonEscapeAppend(j, ssid.c_str());
    }
    j += "\",\"rssi\":";
    j += WiFi.RSSI();
    j += "},\"ap\":{\"active\":";
    j += Settings::apActive() ? "true" : "false";
    j += ",\"ssid\":\"";
    jsonEscapeAppend(j, Settings::apSsid());
    j += "\",\"ip\":\"";
    j += WiFi.softAPIP().toString();
    j += "\",\"url\":\"";
    jsonEscapeAppend(j, setupUrl().c_str());
    j += "\"}},\"settings\":{\"volume\":";
    j += Settings::volume();
    j += ",\"wifiOn\":";
    j += Settings::wifiEnabled() ? "true" : "false";
    j += ",\"ssid\":\"";
    jsonEscapeAppend(j, Settings::wifiSsid());
    j += "\",\"hasPassword\":";
    j += (Settings::wifiPassword()[0] ? "true" : "false");
    j += ",\"tz\":\"";
    jsonEscapeAppend(j, Settings::tzId());
    j += "\",\"ntpOn\":";
    j += Settings::ntpEnabled() ? "true" : "false";
    j += ",\"ntpSynced\":";
    j += Settings::ntpSynced() ? "true" : "false";
    j += "},\"apps\":[";
    for (int i = 0; i < g_appManager.getAppCount(); i++) {
        App *app = g_appManager.getApp(i);
        if (!app) continue;
        AppManifest m = app->getManifest();
        if (i) j += ',';
        bool current = (i == g_appManager.getCurrentIndex());
        bool launcher = m.id && strcmp(m.id, "launcher") == 0;
        j += "{\"id\":\"";
        jsonEscapeAppend(j, m.id);
        j += "\",\"name\":\"";
        jsonEscapeAppend(j, m.name);
        j += "\",\"version\":\"";
        jsonEscapeAppend(j, m.version);
        j += "\",\"author\":\"";
        jsonEscapeAppend(j, m.author);
        j += "\",\"state\":\"";
        j += stateName(app->state);
        j += "\",\"current\":";
        j += current ? "true" : "false";
        j += ",\"killable\":";
        j += launcher ? "false" : "true";
        j += '}';
    }
    j += "]}";
    return j;
}

static void sendAsset(const char *type, PGM_P body) {
    console.sendHeader("Cache-Control", "no-store");
    console.send_P(200, type, body);
}

static void handleRoot() {
    if (WiFi.status() != WL_CONNECTED) {
        console.sendHeader("Location", "/setup");
        console.send(302, "text/plain", "");
        return;
    }
    sendAsset("text/html", WEB_INDEX_HTML);
}

static void handleSetup() {
    sendAsset("text/html", WEB_SETUP_HTML);
}

static void handleCss() {
    sendAsset("text/css", WEB_STYLE_CSS);
}

static void handleJs() {
    sendAsset("application/javascript", WEB_APP_JS);
}

static void handleSetupJs() {
    sendAsset("application/javascript", WEB_SETUP_JS);
}

static void handleStatus() {
    sendJson(200, statusJson());
}

static String scanJson() {
    int n = WiFi.scanComplete();
    String j;
    j.reserve(800);
    if (n == WIFI_SCAN_RUNNING) return F("{\"scanning\":true,\"networks\":[]}");
    if (n < 0) return F("{\"scanning\":false,\"networks\":[]}");
    j += "{\"scanning\":false,\"networks\":[";
    if (n > 12) n = 12;
    for (int i = 0; i < n; i++) {
        if (i) j += ',';
        String ssid = WiFi.SSID(i);
        j += "{\"ssid\":\"";
        jsonEscapeAppend(j, ssid.c_str());
        j += "\",\"rssi\":";
        j += WiFi.RSSI(i);
        j += ",\"open\":";
        j += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "true" : "false";
        j += '}';
    }
    j += "]}";
    WiFi.scanDelete();
    return j;
}

static void handleScanGet() {
    sendJson(200, scanJson());
}

static void handleScanPost() {
    if (WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
        WiFi.scanNetworks(true);
    }
    sendJson(202, F("{\"scanning\":true,\"networks\":[]}"));
}

static void handleOpen() {
    String id = console.arg("id");
    if (id.length() == 0) {
        sendJson(400, F("{\"ok\":false,\"error\":\"id required\"}"));
        return;
    }
    if (g_appManager.indexById(id.c_str()) < 0) {
        sendJson(404, F("{\"ok\":false,\"error\":\"app not found\"}"));
        return;
    }
    g_appManager.switchToAppById(id.c_str());
    sendJson(200, statusJson());
}

static void handleKill() {
    String id = console.arg("id");
    if (id.length() == 0) {
        sendJson(400, F("{\"ok\":false,\"error\":\"id required\"}"));
        return;
    }
    if (id == "launcher") {
        sendJson(400, F("{\"ok\":false,\"error\":\"cannot kill launcher\"}"));
        return;
    }
    if (!g_appManager.stopAppById(id.c_str())) {
        sendJson(404, F("{\"ok\":false,\"error\":\"app not found\"}"));
        return;
    }
    sendJson(200, statusJson());
}

static void handleSettings() {
    if (console.hasArg("volume")) {
        Settings::setVolume((uint8_t)console.arg("volume").toInt());
    }
    if (console.hasArg("wifiOn")) {
        Settings::setWifiEnabled(console.arg("wifiOn") == "1" || console.arg("wifiOn") == "true");
    }
    if (console.hasArg("ssid")) {
        String ssid = console.arg("ssid");
        const char *pass = Settings::wifiPassword();
        String passArg;
        if (console.hasArg("password") && console.arg("password").length() > 0) {
            passArg = console.arg("password");
            pass = passArg.c_str();
        }
        Settings::setWifi(ssid.c_str(), pass);
    }
    if (console.hasArg("tz")) {
        Settings::setTzId(console.arg("tz").c_str());
    }
    if (console.hasArg("ntpOn")) {
        Settings::setNtpEnabled(console.arg("ntpOn") == "1" || console.arg("ntpOn") == "true");
    }
    sendJson(200, statusJson());
}

static void handleInstall() {
    sendJson(501, F("{\"ok\":false,\"error\":\"not_implemented\",\"hint\":\"App sideload / OTA packages are planned\"}"));
}

static void handleRestart() {
    sendJson(200, F("{\"ok\":true}"));
    Power::restart();
}

static void handlePowerOff() {
    sendJson(200, F("{\"ok\":true}"));
    Power::off();
}

static void handleLog() {
    String body;
    LogBuffer::copyTo(body);
    console.sendHeader("Cache-Control", "no-store");
    console.send(200, "text/plain", body);
}

static void beginPortal() {
    if (portalOn || !Settings::apActive()) return;
    dns.start(53, "*", WiFi.softAPIP());
    portal80.onNotFound([]() {
        portal80.sendHeader("Location", setupUrl());
        portal80.send(302, "text/plain", "");
    });
    portal80.begin();
    portalOn = true;
    DEBUG_PRINTF("[WebConsole] captive -> %s\n", setupUrl().c_str());
}

static void beginServer() {
    console.on("/", HTTP_GET, handleRoot);
    console.on("/index.html", HTTP_GET, handleRoot);
    console.on("/setup", HTTP_GET, handleSetup);
    console.on("/setup.html", HTTP_GET, handleSetup);
    console.on("/style.css", HTTP_GET, handleCss);
    console.on("/app.js", HTTP_GET, handleJs);
    console.on("/setup.js", HTTP_GET, handleSetupJs);
    console.on("/api/status", HTTP_GET, handleStatus);
    console.on("/api/wifi/scan", HTTP_GET, handleScanGet);
    console.on("/api/wifi/scan", HTTP_POST, handleScanPost);
    console.on("/api/apps/open", HTTP_POST, handleOpen);
    console.on("/api/apps/kill", HTTP_POST, handleKill);
    console.on("/api/settings", HTTP_POST, handleSettings);
    console.on("/api/apps/install", HTTP_POST, handleInstall);
    console.on("/api/restart", HTTP_POST, handleRestart);
    console.on("/api/poweroff", HTTP_POST, handlePowerOff);
    console.on("/api/log", HTTP_GET, handleLog);
    console.onNotFound([]() {
        if (WiFi.status() != WL_CONNECTED) {
            console.sendHeader("Location", "/setup");
            console.send(302, "text/plain", "");
            return;
        }
        sendJson(404, F("{\"ok\":false,\"error\":\"not found\"}"));
    });
    console.begin();
    started = true;
    DEBUG_PRINTF("[WebConsole] http://%s:%d/\n",
                 WiFi.softAPIP().toString().c_str(), WEB_CONSOLE_PORT);
}

void WebConsole::poll() {
    if (!started) {
        if (!Settings::apActive() && WiFi.status() != WL_CONNECTED) return;
        beginServer();
    }
    beginPortal();
    if (portalOn) {
        dns.processNextRequest();
        portal80.handleClient();
    }
    console.handleClient();
}
