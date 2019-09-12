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
    if(dbg)Serial.println("key (" + key + String(F(") value (")) + value.substring(0, 15) + String(F(") RAM: ")) + ESP.getFreeHeap());
    serializeJson(doc, result);
    config = result;
    pub_mqtt(key, value);
} 

String jeeui2::param(String key) 
{ 
    DynamicJsonDocument doc(10000);
    deserializeJson(doc, config);
    String value = doc[key];
    if(dbg)Serial.print("READ: ");
    if(dbg)Serial.println("key (" + key + String(F(") value (")) + value + ")");
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
        uint8_t params = request->params();
        AsyncWebParameter *p;
        String value = "";
        for (uint8_t i = 0; i < params; i++)
        {
          p = request->getParam(i);
            value = param(p->name());
        }
        request->send(200, F("text/plain"), value);
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
    // rc(m_callback);
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
