#include "Web.h"
#include "config.h"
#include "ui.h"
#include "StepDetector.h"
#include "settings.h"

#include <WiFi.h>
#include <WebServer.h>

void startHunt();
void startDiscover();
void startRest();
void triggerHatch();
void webResetPet();
void saveState();
void applyTftBrightness();
void startWifiScan();
int feedQty(int qty);
int feedSteps(int steps);

static WebServer web(WEB_SERVER_PORT);
static bool webStarted = false;

static const char *moodName(Mood m) {
    switch (m) {
        case MOOD_HUNGRY:  return "HUNGRY";
        case MOOD_HAPPY:   return "HAPPY";
        case MOOD_CURIOUS: return "CURIOUS";
        case MOOD_BORED:   return "BORED";
        case MOOD_SICK:    return "SICK";
        case MOOD_EXCITED: return "EXCITED";
        case MOOD_CALM:    return "CALM";
    }
    return "?";
}

static const char *stageName(Stage s) {
    switch (s) {
        case STAGE_BABY:  return "BABY";
        case STAGE_TEEN:  return "TEEN";
        case STAGE_ADULT: return "ADULT";
        case STAGE_ELDER: return "ELDER";
    }
    return "?";
}

static const char *activityName(Activity a) {
    switch (a) {
        case ACT_HUNT:     return "HUNT";
        case ACT_DISCOVER: return "DISCOVER";
        case ACT_REST:     return "REST";
        default:           return "IDLE";
    }
}

static const char *screenName(Screen s) {
    switch (s) {
        case SCREEN_BOOT:        return "BOOT";
        case SCREEN_HATCH:       return "HATCH";
        case SCREEN_HOME:        return "HOME";
        case SCREEN_MENU:        return "MENU";
        case SCREEN_PET_STATUS:  return "PET_STATUS";
        case SCREEN_ENVIRONMENT: return "ENVIRONMENT";
        case SCREEN_SYSINFO:     return "SYSINFO";
        case SCREEN_CONTROLS:    return "CONTROLS";
        case SCREEN_SETTINGS:    return "SETTINGS";
        case SCREEN_DIAGNOSTICS: return "DIAGNOSTICS";
        case SCREEN_GAMEOVER:    return "GAMEOVER";
    }
    return "?";
}

static const char *wifiDisconnectReason(uint8_t reason) {
    switch (reason) {
        case 2:   return "AUTH_EXPIRE";
        case 15:  return "4WAY_TIMEOUT";
        case 201: return "NO_AP_FOUND";
        case 202: return "AUTH_FAIL";
        case 203: return "ASSOC_FAIL";
        case 204: return "HANDSHAKE_TIMEOUT";
        default:  return "?";
    }
}

static void wifiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            DEBUG_PRINTLN("[WiFi] STA start");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            DEBUG_PRINTF("[WiFi] associated SSID=%s ch=%u\n",
                         WiFi.SSID().c_str(), WiFi.channel());
            break;
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            DEBUG_PRINTF("[WiFi] GOT IP %s gw=%s rssi=%d\n",
                         WiFi.localIP().toString().c_str(),
                         WiFi.gatewayIP().toString().c_str(),
                         WiFi.RSSI());
            DEBUG_PRINTF("[Web] open http://%s/\n", WiFi.localIP().toString().c_str());
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            DEBUG_PRINTF("[WiFi] DISCONNECTED reason=%u (%s)\n",
                         info.wifi_sta_disconnected.reason,
                         wifiDisconnectReason(info.wifi_sta_disconnected.reason));
            break;
        default:
            break;
    }
}

static void jsonEscapeAppend(String &out, const char *s) {
    if (!s) return;
    for (const char *p = s; *p; p++) {
        if (*p == '"' || *p == '\\') { out += '\\'; out += *p; }
        else if ((uint8_t)*p >= 32) out += *p;
    }
}

static String wifiJson() {
    String j;
    j.reserve(800);
    j += "{\"scanning\":";
    j += wifiScanInProgress ? "true" : "false";
    j += ",\"connected\":";
    j += (WiFi.status() == WL_CONNECTED) ? "true" : "false";
    j += ",\"ip\":\"";
    j += WiFi.localIP().toString();
    j += "\",\"ssid\":\"";
    jsonEscapeAppend(j, WiFi.SSID().c_str());
    j += "\",\"rssi\":";
    j += WiFi.RSSI();
    j += ",\"lastScanMs\":";
    j += lastWifiScanTime;
    j += ",\"netCount\":";
    j += wifiStats.netCount;
    j += ",\"strongCount\":";
    j += wifiStats.strongCount;
    j += ",\"hiddenCount\":";
    j += wifiStats.hiddenCount;
    j += ",\"openCount\":";
    j += wifiStats.openCount;
    j += ",\"avgRSSI\":";
    j += wifiStats.avgRSSI;
    j += ",\"networks\":[";
    for (uint8_t i = 0; i < wifiNetStored; i++) {
        if (i) j += ',';
        j += "{\"ssid\":\"";
        jsonEscapeAppend(j, wifiNets[i].ssid);
        j += "\",\"rssi\":";
        j += wifiNets[i].rssi;
        j += ",\"open\":";
        j += wifiNets[i].open ? "true" : "false";
        j += ",\"hidden\":";
        j += wifiNets[i].hidden ? "true" : "false";
        j += '}';
    }
    j += "]}";
    return j;
}

static String petJson() {
    String j;
    j.reserve(900);
    j += "{\"hunger\":";
    j += pet.hunger;
    j += ",\"happiness\":";
    j += pet.happiness;
    j += ",\"health\":";
    j += pet.health;
    j += ",\"ageDays\":";
    j += pet.ageDays;
    j += ",\"ageHours\":";
    j += pet.ageHours;
    j += ",\"ageMinutes\":";
    j += pet.ageMinutes;
    j += ",\"mood\":\"";
    j += moodName(currentMood);
    j += "\",\"stage\":\"";
    j += stageName(petStage);
    j += "\",\"activity\":\"";
    j += activityName(currentActivity);
    j += "\",\"screen\":\"";
    j += screenName(currentScreen);
    j += "\",\"hatched\":";
    j += hasHatchedOnce ? "true" : "false";
    j += ",\"step\":{\"total\":";
    j += StepDetector::total();
    j += ",\"remainder\":";
    j += StepDetector::remainder();
    j += ",\"stepsPerFeed\":";
    j += StepDetector::stepsPerFeed();
    j += ",\"hungerPerFeed\":";
    j += HUNGER_PER_FEED;
    j += ",\"multiplier\":";
    j += StepDetector::multiplier();
    j += ",\"imu\":";
    j += StepDetector::imuReady() ? "true" : "false";
    j += "}";
    j += ",\"wifi\":";
    j += wifiJson();
    j += ",\"traits\":{\"curiosity\":";
    j += traitCuriosity;
    j += ",\"activity\":";
    j += traitActivity;
    j += ",\"stress\":";
    j += traitStress;
    j += "},\"settings\":{\"sound\":";
    j += soundEnabled ? "true" : "false";
    j += ",\"brightness\":";
    j += Settings::brightness();
    j += "}}";
    return j;
}

static void handleRoot() {
    String html;
    html.reserve(4200);
    html += F("<!DOCTYPE html><html><head><meta charset=utf-8>"
              "<meta name=viewport content='width=device-width,initial-scale=1'>"
              "<title>TamaFi</title>"
              "<style>body{font-family:sans-serif;background:#1a1a2e;color:#eee;margin:24px}"
              "button,input{font-size:16px;padding:10px 14px;margin:6px;border:0;border-radius:10px}"
              "button{color:#fff}.h{background:#e74c3c}.d{background:#e67e22}.r{background:#9b59b6}"
              ".k{background:#3498db}.g{background:#27ae60}"
              "input{background:#333;color:#eee}"
              "bar{display:block;background:#333;height:12px;border-radius:6px;margin:4px 0 12px}"
              "bar>i{display:block;height:100%;border-radius:6px}"
              "pre{background:#111;padding:10px;border-radius:8px;overflow:auto}</style></head><body>");
    html += "<h1>TamaFi</h1><p id=meta>";
    html += stageName(petStage);
    html += " | ";
    html += moodName(currentMood);
    html += " | ";
    html += activityName(currentActivity);
    html += " | ";
    html += String(pet.ageDays);
    html += "d ";
    html += String(pet.ageHours);
    html += "h ";
    html += String(pet.ageMinutes);
    html += "m</p><p>Hunger <b id=ht>";
    html += String(pet.hunger);
    html += "</b></p><bar><i id=hb style='width:";
    html += String(pet.hunger);
    html += "%;background:#e74c3c'></i></bar><p>Happy <b id=yt>";
    html += String(pet.happiness);
    html += "</b></p><bar><i id=yb style='width:";
    html += String(pet.happiness);
    html += "%;background:#f1c40f'></i></bar><p>Health <b id=lt>";
    html += String(pet.health);
    html += "</b></p><bar><i id=lb style='width:";
    html += String(pet.health);
    html += "%;background:#2ecc71'></i></bar>";
    html += F("<form id=fq>qty "
              "<input name=qty type=number min=1 max=100 value=10>"
              "<button class=h>Feed qty</button></form>"
              "<form id=fs>step "
              "<input name=step type=number min=1 value=1000>"
              "<button class=g>Feed steps</button></form>"
              "<form method=post action=/api/wifi/scan>"
              "<button class=k>WiFi scan</button></form>"
              "<form method=post action=/hunt><button class=h>Hunt WiFi</button></form>"
              "<form method=post action=/discover><button class=d>Discover</button></form>"
              "<form method=post action=/rest><button class=r>Rest</button></form>"
              "<form method=post action=/hatch><button class=k>Hatch</button></form>"
              "<form method=post action=/reset><button class=h>Reset</button></form>"
              "<pre id=out></pre>"
              "<p><a href=/api/pet>JSON pet</a> · <a href=/api/wifi>JSON wifi</a></p>"
              "<script>"
              "function setBar(id,v){"
                "if(v==null)return;"
                "document.getElementById(id+'t').textContent=v;"
                "document.getElementById(id+'b').style.width=v+'%';"
              "}"
              "function apply(j){"
                "if(!j)return;"
                "setBar('h',j.hunger);setBar('y',j.happiness);setBar('l',j.health);"
                "if(j.stage||j.mood||j.activity){"
                  "var a=j.ageDays||0,h=j.ageHours||0,m=j.ageMinutes||0;"
                  "document.getElementById('meta').textContent="
                    "(j.stage||'')+' | '+(j.mood||'')+' | '+(j.activity||'')+' | '+a+'d '+h+'h '+m+'m';"
                "}"
              "}"
              "async function call(url){"
                "var r=await fetch(url,{method:'POST'});"
                "var j=await r.json();"
                "document.getElementById('out').textContent=JSON.stringify(j);"
                "apply(j);"
                "return j;"
              "}"
              "document.getElementById('fq').onsubmit=function(e){"
                "e.preventDefault();"
                "call('/api/feed?qty='+encodeURIComponent(e.target.qty.value));"
              "};"
              "document.getElementById('fs').onsubmit=function(e){"
                "e.preventDefault();"
                "call('/api/feed?step='+encodeURIComponent(e.target.step.value));"
              "};"
              "async function refresh(){"
                "try{apply(await(await fetch('/api/pet')).json())}catch(e){}"
              "}"
              "setInterval(refresh,2000);"
              "</script></body></html>");
    web.sendHeader("Cache-Control", "no-store");
    web.send(200, "text/html", html);
}

static void sendJson(int code, const String &body) {
    web.send(code, "application/json", body);
}

static void handleJson() {
    sendJson(200, petJson());
}

static void handleWifiGet() {
    sendJson(200, wifiJson());
}

static void handleWifiScan() {
    if (!wifiScanInProgress) startWifiScan();
    sendJson(202, wifiJson());
}

static int argInt(const char *name, int fallback = -1) {
    if (!web.hasArg(name)) return fallback;
    return web.arg(name).toInt();
}

static void handleFeed() {
    int step = argInt("step");
    int qty = argInt("qty");
    if (step < 0 && qty < 0) {
        sendJson(400, F("{\"ok\":false,\"error\":\"qty or step required\"}"));
        return;
    }

    int applied;
    String j;
    j.reserve(280);
    if (step >= 0) {
        applied = feedSteps(step);
        j += "{\"ok\":true,\"source\":\"step\",\"step\":";
        j += step;
        j += ",\"stepsPerFeed\":";
        j += StepDetector::stepsPerFeed();
        j += ",\"hungerPerFeed\":";
        j += HUNGER_PER_FEED;
        j += ",\"multiplier\":";
        j += StepDetector::multiplier();
        j += ",\"total\":";
        j += StepDetector::total();
        j += ",\"remainder\":";
        j += StepDetector::remainder();
    } else {
        applied = feedQty(qty);
        j += "{\"ok\":true,\"source\":\"qty\",\"qty\":";
        j += qty;
    }
    j += ",\"applied\":";
    j += applied;
    j += ",\"hunger\":";
    j += pet.hunger;
    j += ",\"happiness\":";
    j += pet.happiness;
    j += ",\"health\":";
    j += pet.health;
    j += '}';
    sendJson(200, j);
}

static void redirectHome() {
    web.sendHeader("Location", "/");
    web.send(303);
}

void wifiBegin() {
    if (!Settings::wifiEnabled()) {
        DEBUG_PRINTLN("[WiFi] disabled in Settings");
        return;
    }
    const char *ssid = Settings::hasWifiConfig() ? Settings::wifiSsid() : WIFI_SSID;
    const char *pass = Settings::hasWifiConfig() ? Settings::wifiPassword() : WIFI_PASSWORD;
    if (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssid) {
        DEBUG_PRINTLN("[WiFi] already connected");
        return;
    }
    DEBUG_PRINTF("[WiFi] begin SSID='%s' (password %s)\n",
                 ssid, (pass && pass[0] ? "set" : "EMPTY"));
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.onEvent(wifiEvent);
    WiFi.begin(ssid, pass);
}

void webBegin() {
    web.on("/", HTTP_GET, handleRoot);
    web.on("/api/pet", HTTP_GET, handleJson);
    web.on("/api/wifi", HTTP_GET, handleWifiGet);
    web.on("/api/wifi/scan", HTTP_GET, handleWifiGet);
    web.on("/api/wifi/scan", HTTP_POST, handleWifiScan);
    web.on("/api/feed", HTTP_GET, handleFeed);
    web.on("/api/feed", HTTP_POST, handleFeed);
    web.on("/hunt", HTTP_POST, []() { startHunt(); redirectHome(); });
    web.on("/discover", HTTP_POST, []() { startDiscover(); redirectHome(); });
    web.on("/rest", HTTP_POST, []() { startRest(); redirectHome(); });
    web.on("/hatch", HTTP_POST, []() { triggerHatch(); redirectHome(); });
    web.on("/reset", HTTP_POST, []() { webResetPet(); redirectHome(); });
    web.onNotFound([]() {
        DEBUG_PRINTF("[Web] 404 %s\n", web.uri().c_str());
        web.send(404, "text/plain", "not found");
    });
    web.begin();
    webStarted = true;
    DEBUG_PRINTF("[Web] HTTP server on port %d\n", WEB_SERVER_PORT);
}

void webLoop() {
    if (webStarted) web.handleClient();
}
