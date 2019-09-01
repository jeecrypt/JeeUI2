/* 
 
*/ 

#include "JeeUI2.h"
#include "pge.h"
#include "stl.h"
#include "mnu.h"
#include "rng.h"
#include "mkr.h"
#include "mui.h"
#include "grids.h"

#include "temp_js.h"

AsyncWebServer server(80);
// StaticJsonDocument<18000> doc;

void jeeui2::var(String key, String value) 
{ 
    DynamicJsonDocument doc(10000);
    String result;
    deserializeJson(doc, config);
    JsonObject obj = doc.as<JsonObject>();
    obj[key] = value;
    if(dbg)Serial.print("WRITE: ");
    if(dbg)Serial.println("key (" + key + ") value (" + value.substring(0, 15) + ")");
    serializeJson(doc, result);
    config = result;

} 

String jeeui2::param(String key) 
{ 
    DynamicJsonDocument doc(10000);
    deserializeJson(doc, config);
    String value = doc[key];
    if(dbg)Serial.print("READ: ");
    if(dbg)Serial.println("key (" + key + ") value (" + value + ")");
    return value;
    serializeJson(doc, config);
} 

void jeeui2::deb() 
{ 
    if(dbg)Serial.print("CONFIG: ");
    if(dbg)Serial.println(config);
}

void jeeui2::begin(bool debug) {
    dbg = debug;
    nonWifiVar();
    load();
    if(dbg)Serial.println("CONFIG: " + config);
    begin();
    if(dbg)Serial.println("RAM: " + String(ESP.getFreeHeap()));
    if(dbg)Serial.println("MAC: " + mac);
}

void jeeui2::begin() { 
    
    
    wifi_connect();
    server.on("/post", HTTP_ANY, [this](AsyncWebServerRequest *request) {
        uint8_t params = request->params();
        AsyncWebParameter *p;
        for (uint8_t i = 0; i < params; i++)
        {
          p = request->getParam(i);
          if (p->name().indexOf("BTN_") != -1) btnui = p->name().substring(4, p->name().length());
          else {
            var(p->name(), p->value());
            as();
          }
        }
        
        request->send(200, F("text/plain"), F("OK"));
    });

    server.on("/echo", HTTP_ANY, [this](AsyncWebServerRequest *request) { 
        foo();
        request->send(200, F("text/plain"), buf);
        buf = "";
    });

    server.on("/config", HTTP_ANY, [this](AsyncWebServerRequest *request) { 
        request->send(200, F("text/plain"), config);
    });

    server.on("/js/maker.js", HTTP_ANY, [this](AsyncWebServerRequest *request) {
        request->send_P(200, F("application/javascript"), temp_js);
    });

    server.on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/html"), pge, pge_l);
        response->addHeader(F("Content-Encoding"), F("gzip"));
        request->send(response);
    });

    server.on("/css/pure-min.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/css"), stl, stl_l);
        response->addHeader(F("Content-Encoding"), F("gzip"));
        request->send(response);
    });

    server.on("/css/side-menu.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/css"), mnu, mnu_l);
        response->addHeader(F("Content-Encoding"), F("gzip"));
        request->send(response);
    });

    server.on("/css/range.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/css"), rng, rng_l);
        response->addHeader(F("Content-Encoding"), F("gzip"));
        request->send(response);
    });

    server.on("/css/grids.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/css"), grids, grids_l);
        response->addHeader(F("Content-Encoding"), F("gzip"));
        request->send(response);
    });

    server.on("/js/ui.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, F("application/javascript"), mui, mui_l);
        response->addHeader(F("Content-Encoding"), F("gzip"));
        request->send(response);
    });

    // server.on("/js/maker.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    //     AsyncWebServerResponse *response = request->beginResponse_P(200, F("application/javascript"), mkr, mkr_l);
    //     response->addHeader(F("Content-Encoding"), F("gzip"));
    //     request->send(response);
    // });

    server.begin();
    
    upd();
}

void jeeui2::led(uint8_t pin, bool invert)
{
    if (pin == -1) return;
    LED_PIN = pin;
    LED_INVERT = invert;
    pinMode(LED_PIN, OUTPUT);
}

void jeeui2::handle()
{
    udpLoop();
    static unsigned long timer;
    unsigned int interval = 300;
    if (timer + interval > millis())
        return;
    timer = millis();
    btn();
    led_handle();
    autosave();
}


void jeeui2::nonWifiVar(){
    #ifdef ESP32
    WiFi.mode(WIFI_MODE_AP);
    #else
    WiFi.mode(WIFI_AP);
    #endif
    String mc = String(WiFi.softAPmacAddress());
    mc.replace(":", "");
    String wifi = param("wifi");
    String ssid = param("ssid");
    String pass = param("pass");
    String ap_ssid = param("ap_ssid");
    String ap_pass = param("ap_pass");
    if(wifi == "null") var("wifi", "AP");
    if(ssid == "null") var("ssid", "JeeUI2");
    if(pass == "null") var("pass", "");
    if(ap_ssid == "null") var("ap_ssid", "JeeUI2-" + mc);
    if(ap_pass == "null") var("ap_pass", "");
}

