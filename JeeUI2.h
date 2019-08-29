#ifndef JeeUI2_h
#define JeeUI2_h

#include "Arduino.h"

#ifdef ESP8266
#include <ESPAsyncTCP.h>
#include <FS.h>
#else
#include <AsyncTCP.h>
#include "SPIFFS.h"
#endif

#include <ESPAsyncWebServer.h>
#include "ArduinoJson.h"

#define BUTTON 0

class jeeui2
{
    typedef void (*buttonCallback) ();
    typedef void (*uiCallback) ();
    typedef void (*updateCallback) ();
    //typedef void (*uiFunction) ();
    

  public:
    void var(String key, String value);
    String param(String key);
    void led(uint8_t pin, bool invert);
    void deb();
    void ap(unsigned long interval);
    void begin();
    void begin(bool debug);
    void handle();
    void btn();
    void btnCallback(String name ,buttonCallback response);

    void app(String name);
    void menu(String name);
    void page();

    void text(String id, String label);
    void password(String id, String label);
    void number(String id, String label);
    void time(String id, String label);
    void date(String id, String label);
    void datetime(String id, String label);
    void email(String id, String label);
    void range(String id, int min, int max, float step, String label);
    void select(String id, String label);
    void option(String value, String label);
    void checkbox(String id, String label);
    void color(String id, String label);
    void button(String id, String color, String label);
    void button(String id, String color, String label, int column);
    void textarea(String id, String label);
    void save();
    
    uiCallback foo;
    void ui(void (*uiFunction) ());

    updateCallback upd;
    void update(void (*updateFunction) ());

    String ip;
    String buf;
    String mac;
    bool connected = false;


  private:
    void arr(String key, String value);
    void wifi_connect();
    void button_handle();
    void led_handle();
    void nonWifiVar();
    void led_on();
    void led_off();
    void led_inv();
    void load();
    void autosave();
    void as();
    void prntmac();
    void udpBegin();
    void udpLoop();

    String udpRemoteIP;
    unsigned int localUdpPort = 4243;
    char incomingPacket[64];

    unsigned int asave = 1000;
    bool sv = false;
    unsigned long astimer;

    String config = "{}";
    String op = "";
    uint8_t wifi_mode;
    int LED_PIN = -1;
    bool LED_INVERT = false;
    uint8_t mn = 0;
    unsigned long a_ap = 0;
    bool wf = false;
    uint8_t pg = 0;
    String btnui = "";
    
    bool dbg = false;
    
};



#endif