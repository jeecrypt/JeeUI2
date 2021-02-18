#include "JeeUI2.h"

void jeeui2::save()
{
    if(LittleFS.begin()){
    }
    File configFile = LittleFS.open(F("/config.json"), "w");
    
    String cfg_str;
    serializeJson(cfg, cfg_str);
    deserializeJson(cfg, cfg_str);
    configFile.print(cfg_str);
    cfg_str = "";

    if(dbg)Serial.println(F("Save Config"));
}

void jeeui2::autosave(){
    
    if (!sv) return;
    if (sv && astimer + asave < millis()){
        save();
        sv = false;
        if(dbg)Serial.println("AutoSave");
        upd();
        mqtt_update();
    } 
}

void jeeui2::update(void (*updateFunction) ())
{
    upd = updateFunction;
}

void jeeui2::as(){
    sv = true;
    astimer = millis();
}

void jeeui2::load()
{
    if(LittleFS.begin()){
    }
    File pre_configFile = LittleFS.open(F("/config.json"), "r");
    if (pre_configFile.readString() == "")
    {
        if(dbg)Serial.println(F("Failed to open config file"));
        save();
        return;
    }
    File configFile = SPIFFS.open(F("/config.json"), "r");
    String cfg_str = configFile.readString();
    deserializeJson(cfg, cfg_str);
    if(dbg)Serial.println(F("JSON config loaded"));
}
