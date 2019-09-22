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
#include "chk.h"

#include "temp_js.h"

AsyncWebServer server(80);

void jeeui2::var(String key, String value) 
{ 
    if(pub_enable){
        JsonVariant pub_key = pub_transport[key];
        if (!pub_key.isNull()) {
            pub_transport[key] = value;
            if(dbg)Serial.println("Pub: [" + key + " - " + value + "]");
            pub_mqtt(key, value);
            String tmp;
            serializeJson(pub_transport, tmp);
            deserializeJson(pub_transport, tmp);
            tmp = "";
            return;
        }
    }
    if(rc)publish("jee/set/" + key, value, true);
    if(dbg)Serial.print("WRITE: ");
    if(dbg)Serial.println("key (" + key + String(F(") value (")) + value.substring(0, 15) + String(F(") RAM: ")) + ESP.getFreeHeap());
    cfg[key] = value;
} 

String jeeui2::param(String key) 
{ 
    String value = cfg[key];
    if(dbg)Serial.print("READ: ");
    if(dbg)Serial.println("key (" + key + String(F(") value (")) + value + ")");
    return value;
} 

String jeeui2::deb() 
{ 
    String cfg_str;
    serializeJson(cfg, cfg_str);
    return cfg_str;  
}

void jeeui2::begin(bool debug) {
    dbg = debug;
    nonWifiVar();
    load();
    if(dbg)Serial.println("CONFIG: " + deb());
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
          if (p->name().indexOf("BTN_") != -1){
                btnui = p->name().substring(4, p->name().length());
                if(btnui == "bWF"){
                    var("wifi", "STA");
                    save();
                    ESP.restart();
                }
          } 
          else {
            var(p->name(), p->value());
            as();
          }
        }
        request->send(200, F("text/plain"), F("OK"));
    });

    server.on("/pub", HTTP_ANY, [this](AsyncWebServerRequest *request) {
        AsyncWebParameter *p;
        String value = "";
        p = request->getParam(0);
        JsonVariant pub_key = pub_transport[p->name()];
        if (!pub_key.isNull()) {
            value = pub_transport[p->name()].as<String>();
            if(dbg)Serial.println("pub: [" + p->name() + " - " + value + "]");

        }
        request->send(200, F("text/plain"), value);
    });

    server.on("/echo", HTTP_ANY, [this](AsyncWebServerRequest *request) { 
        foo();
        request->send(200, F("text/plain"), buf);
        buf = "";
    });

    server.on("/config", HTTP_ANY, [this](AsyncWebServerRequest *request) { 
        String config = deb();
        request->send(200, F("text/plain"), config);
        config = "";
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

    server.on("/grids.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/css"), grids, grids_l);
        response->addHeader(F("Content-Encoding"), F("gzip"));
        request->send(response);
    });

    server.on("/chk.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, F("text/css"), chk, chk_l);
        response->addHeader(F("Content-Encoding"), F("gzip"));
        request->send(response);
    });

    server.on("/js/ui.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse_P(200, F("application/javascript"), mui, mui_l);
        response->addHeader(F("Content-Encoding"), F("gzip"));
        request->send(response);
    });
    server.begin();
    foo();
    upd();
    mqtt_update();
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
    _connected();
    mqtt_handle();
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
    getAPmac();
    String wifi = param(F("wifi"));
    String ssid = param(F("ssid"));
    String pass = param(F("pass"));
    String ap_ssid = param(F("ap_ssid"));
    String ap_pass = param(F("ap_pass"));
    if(wifi == F("null")) var(F("wifi"), F("AP"));
    if(ssid == F("null")) var(F("ssid"), F("JeeUI2"));
    if(pass == F("null")) var(F("pass"), "");
    if(ap_ssid == F("null")) var(F("ap_ssid"), String(F("JeeUI2-")) + mc);
    if(ap_pass == F("null")) var(F("ap_pass"), "");
}

void jeeui2::getAPmac(){
    if(mc != "") return;
    #ifdef ESP32
    WiFi.mode(WIFI_MODE_AP);
    #else
    WiFi.mode(WIFI_AP);
    #endif
    mc = String(WiFi.softAPmacAddress());
    mc.replace(":", "");
}
