#include "JeeUI2.h"

void jeeui2::save()
{
    if(SPIFFS.begin()){
    }
    File configFile = SPIFFS.open(F("/config.json"), "w");
    configFile.print(config);
    if(dbg)Serial.println(F("Save Config"));
}

void jeeui2::autosave(){
    
    if (!sv) return;
    if (sv && astimer + asave < millis()){
        save();
        sv = false;
        if(dbg)Serial.println("AutoSave");
        upd();
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
    if(SPIFFS.begin()){
    }
    File pre_configFile = SPIFFS.open(F("/config.json"), "r");
    if (pre_configFile.readString() == "")
    {
        if(dbg)Serial.println(F("Failed to open config file"));
        save();
        return;
    }
    File configFile = SPIFFS.open(F("/config.json"), "r");
    config = configFile.readString();
    if(dbg)Serial.println(F("JSON config loaded"));
}